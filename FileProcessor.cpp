#include "FileProcessor.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QThread>

FileProcessor::FileProcessor(QObject *parent)
    : QObject(parent)
{
}

void FileProcessor::setFolder(const QString &inputDir,
                              const QString &mask,
                              const QByteArray &key,
                              const QString &outDir,
                              bool overwrite,
                              bool deleteInput)
{
    inputDirPath = inputDir;
    fileMask = mask;
    keyBytes = key;
    outputDir = outDir;
    overwriteExisting = overwrite;
    deleteInputFiles = deleteInput;
}

void FileProcessor::process()
{
    QDir dir(inputDirPath);
    QStringList files = dir.entryList({fileMask}, QDir::Files);
    if (files.isEmpty()) {
        emit finished(false, "Нет файлов, соответствующих маске!");
        return;
    }

    // Считаем суммарный размер всех файлов
    qint64 totalSize = 0;
    for (const QString &f : files) {
        QFileInfo fi(dir.filePath(f));
        totalSize += fi.size();
    }

    qint64 processedTotal = 0;
    const qint64 bufferSize = 1024 * 1024; // 1 МБ

    for (const QString &fileName : files) {
        QString inputPath = dir.filePath(fileName);
        QFile inFile(inputPath);
        if (!inFile.open(QIODevice::ReadOnly)) {
            emit finished(false, "Не удалось открыть файл: " + fileName);
            continue;
        }

        QFileInfo fi(inputPath);
        QString outPath;

        if (overwriteExisting) {
            outPath = inputPath;
        } else {
            outPath = QDir(outputDir).filePath(fi.fileName());
            int counter = 1;
            while (QFile::exists(outPath)) {
                QString baseName = fi.completeBaseName();
                QString suffix = fi.suffix().isEmpty() ? "txt" : fi.suffix();
                outPath = QDir(outputDir).filePath(QString("%1_%2.%3")
                                                       .arg(baseName)
                                                       .arg(counter++)
                                                       .arg(suffix));
            }
        }

        QFile outFile(outPath);
        if (!outFile.open(QIODevice::WriteOnly)) {
            emit finished(false, "Не удалось записать файл: " + fileName);
            inFile.close();
            continue;
        }

        while (!inFile.atEnd()) {
            QByteArray buffer = inFile.read(bufferSize);
            for (int i = 0; i < buffer.size(); ++i)
                buffer[i] ^= keyBytes[i % keyBytes.size()];
            outFile.write(buffer);

            processedTotal += buffer.size();
            int percent = static_cast<int>((processedTotal * 100) / totalSize);
            emit progress(percent);
        }

        inFile.close();
        outFile.close();

        if (!overwriteExisting && deleteInputFiles) {
            QFile::remove(inputPath);
        }
    }

    emit progress(100);
    emit finished(true, "Все файлы успешно обработаны!");
}
