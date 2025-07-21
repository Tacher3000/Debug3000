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

QString FileController::runScript(const QString& filePath) {
    QString output = runner->runDebugScript(filePath);
    QString outputPath = QCoreApplication::applicationDirPath() + "/out.txt";
    QFile outputFile(outputPath);
    if (!outputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Не удалось открыть файл вывода:" << outputPath << "-" << outputFile.errorString();
        return QString();
    }
    QTextStream in(&outputFile);
    in.setEncoding(QStringConverter::Utf8);
    QString content = in.readAll();
    outputFile.close();
    return content;
}
