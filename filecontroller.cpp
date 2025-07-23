#include "filecontroller.h"
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QCoreApplication>

FileController::FileController(QObject* parent) : QObject(parent) {
    processor = new FileProcessor(this);
    runner = new ScriptRunner(this);
}

FileController::~FileController() {
    delete processor;
    delete runner;
}

QString FileController::openFile(const QString& path) {
    if (path.endsWith(".txt")) {
        QFile file(path);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString content = in.readAll();
            file.close();
            return content;
        }
        return processor->readTxtFile(path);
    } else if (path.endsWith(".COM") || path.endsWith(".com")) {
        return processor->readComFile(path);
    }
    return QString();
}

bool FileController::saveFile(const QString& path, const QString& content) {
    if (path.endsWith(".txt")) {
        return processor->saveTxtFile(path, content);
    }
    return false;
}

bool FileController::saveAsFile(const QString& path, const QString& content) {
    if (path.endsWith(".txt")) {
        return processor->saveTxtFile(path, content);
    }
    return false;
}

QString FileController::pasteCodeToDebug(const QString& filePath) {
    return runner->pasteCodeToDebug(filePath);
}

QString FileController::compileAndRunCom(const QString& filePath) {
    return runner->compileAndRunCom(filePath);
}
