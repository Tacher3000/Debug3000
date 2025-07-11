#ifndef FILEPROCESSOR_H
#define FILEPROCESSOR_H

#include <QObject>
#include <QString>

class FileProcessor : public QObject {
    Q_OBJECT
public:
    FileProcessor(QObject* parent = nullptr);
    ~FileProcessor();
    QString readTxtFile(const QString& path);
    QString readComFile(const QString& path);
    bool saveTxtFile(const QString& path, const QString& content);
};

#endif // FILEPROCESSOR_H
