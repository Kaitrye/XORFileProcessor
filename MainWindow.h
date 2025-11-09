#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onSelectInputClicked();
    void onProcessClicked();
    void onBrowseOutputClicked();
    void startProcessing(char timer = 0);

private:
    Ui::MainWindow *ui;
    QString inputDirPath;
    QString outputDirPath;
    QTimer *fileCheckTimer;
    bool isProcessing = false;
};

#endif // MAINWINDOW_H
