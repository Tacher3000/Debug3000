#ifndef FILEPROCESSOR_H
#define FILEPROCESSOR_H

class QString;

class FileProcessor {
public:
    FileProcessor();
    ~FileProcessor();
    QString readTxtFile(const QString& path);
    QString readComFile(const QString& path);
    bool saveTxtFile(const QString& path, const QString& content);
};

#endif // FILEPROCESSOR_H
