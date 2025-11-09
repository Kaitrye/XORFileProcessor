#ifndef FILEPROCESSOR_H
#define FILEPROCESSOR_H

#include <QObject>
#include <QByteArray>
#include <QString>

class FileProcessor : public QObject
{
    Q_OBJECT
public:
    explicit FileProcessor(QObject *parent = nullptr);

    void setFolder(const QString &inputDir,
                   const QString &mask,
                   const QByteArray &key,
                   const QString &outputDir,
                   bool overwriteExisting,
                   bool deleteInputFiles);

public slots:
    void process();

signals:
    void progress(int percent);        // общий прогресс по всем файлам
    void finished(bool success, const QString &msg);

private:
    QString inputDirPath;
    QString fileMask;
    QByteArray keyBytes;
    QString outputDir;
    bool overwriteExisting = false;
    bool deleteInputFiles = false;
};

#endif // FILEPROCESSOR_H
