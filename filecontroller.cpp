#include "filecontroller.h"
#include <QFile>
#include <QTextStream>

FileController::FileController() {
    processor = new FileProcessor();
    runner = new ScriptRunner();
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
    }
    return QString();
}

bool FileController::saveFile(const QString& path, const QString& content) { return false; }
QString FileController::runScript(const QString& filePath) { return QString(); }
