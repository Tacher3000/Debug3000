#ifndef FILECONTROLLER_H
#define FILECONTROLLER_H

#include <QString>
#include "fileprocessor.h"
#include "scriptrunner.h"

class FileController : public QObject {
    Q_OBJECT
public:
    FileController(QObject* parent = nullptr);
    ~FileController();
    QString openFile(const QString& path);
    bool saveFile(const QString& path, const QString& content);
    bool saveAsFile(const QString& path, const QString& content);
    QString pasteCodeToDebug(const QString& filePath);
    QString compileAndRunCom(const QString& filePath);
private:
    FileProcessor* processor;
    ScriptRunner* runner;
};

#endif // FILECONTROLLER_H
