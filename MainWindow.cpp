#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "FileProcessor.h"
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QThread>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->dateEdit->setEnabled(false);
    ui->timeEdit->setEnabled(false);

    connect(ui->selectInputButton, &QPushButton::clicked, this, &MainWindow::onSelectInputClicked);
    connect(ui->processButton, &QPushButton::clicked, this, &MainWindow::onProcessClicked);
    connect(ui->browseOutputButton, &QPushButton::clicked, this, &MainWindow::onBrowseOutputClicked);
    connect(ui->overWriteCheckBox, &QCheckBox::toggled, this, [=](bool checked){
        ui->deleteInputCheckBox->setEnabled(!checked);
    });
    connect(ui->timerCheckBox, &QCheckBox::toggled, this, [=](bool checked) {
        ui->dateEdit->setEnabled(checked);
        ui->timeEdit->setEnabled(checked);
    });
    connect(ui->stopTimerButton, &QPushButton::clicked, this, [=]() {
        if (fileCheckTimer->isActive()) {
            fileCheckTimer->stop();
            ui->processButton->setEnabled(true);
            ui->stopTimerButton->setEnabled(false);
            QMessageBox::information(this, "Таймер", "Режим таймера остановлен.");
        }
    });

    fileCheckTimer = new QTimer(this);
    fileCheckTimer->setSingleShot(false);

    // Подключаем таймер к обработке файлов
    connect(fileCheckTimer, &QTimer::timeout, this, [=](){
        if (!isProcessing) {
            startProcessing(2);
        }
    });

    ui->progressBar->setValue(0);
    setWindowIcon(QIcon(":/icons/electrical.png"));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onSelectInputClicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Выберите папку для обработки");
    if (!dir.isEmpty()) {
        inputDirPath = dir;
        ui->inputDirLineEdit->setText(dir);
    }
}

void MainWindow::onProcessClicked()
{
    if (inputDirPath.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Выберите папку для обработки!");
        return;
    }

    startProcessing(ui->timerCheckBox->isChecked() ? 1 : 0);
}

void MainWindow::startProcessing(char timer)
{
    QString keyText = ui->keyLineEdit->text();
    if (keyText.length() != 8) {
        QMessageBox::warning(this, "Ошибка", "Введите ровно 8 символов ключа!");
        return;
    }

    QByteArray keyBytes = keyText.toUtf8();

    bool overwrite = ui->overWriteCheckBox->isChecked();
    bool deleteInput = ui->deleteInputCheckBox->isChecked();

    // Маска файлов (*.bin, *.txt и т.д.)
    QString fileMask = ui->maskLineEdit->text().trimmed();
    if (fileMask.isEmpty()) {
        fileMask = "*";
    }

    // Папка для сохранения результатов
    if (outputDirPath.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Выберите папку для сохранения!");
        return;
    }

    ui->processButton->setEnabled(false);

    // Создаём поток и обработчик
    QThread *thread = new QThread;
    FileProcessor *worker = new FileProcessor;
    worker->setFolder(inputDirPath, fileMask, keyBytes, outputDirPath, overwrite, deleteInput);
    worker->moveToThread(thread);

    // Подключаем сигналы
    connect(thread, &QThread::started, worker, &FileProcessor::process);
    connect(worker, &FileProcessor::progress, ui->progressBar, &QProgressBar::setValue);
    connect(worker, &FileProcessor::finished, this, [=](bool success, const QString &msg){
        if (!success){
            QMessageBox::information(this, "Ошибка", msg);
        }
        else if (timer == 0){
            QMessageBox::information(this, "Готово", msg);
            ui->processButton->setEnabled(true);
        }
        isProcessing = false;
        thread->quit();
    });
    connect(worker, &FileProcessor::finished, worker, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);

    isProcessing = true;
    thread->start();

    if (timer == 1) {
        // Получаем интервал из QTimeEdit
        QTime d = ui->dateEdit->time();
        QTime t = ui->timeEdit->time();
        int intervalMs = d.minute()*86400*1000 + t.hour()*3600*1000 + t.minute()*60*1000 + t.second()*1000;

        fileCheckTimer->start(intervalMs);  // запускаем таймер
        ui->stopTimerButton->setEnabled(true);
        QMessageBox::information(this, "Таймер",
                                 "Режим таймера активирован.");
    }
}

void MainWindow::onBrowseOutputClicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Выберите папку для сохранения"));
    if (!dir.isEmpty()) {
        outputDirPath = dir;
        ui->outputDirLineEdit->setText(dir);
    }
}
