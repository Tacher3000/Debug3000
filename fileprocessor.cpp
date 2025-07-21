#include "fileprocessor.h"
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QDir>
#include <QCoreApplication>
#include <QDebug>
#include <QStringDecoder>
#include <QFileInfo>
#include <QRegularExpression>
#include "scriptrunner.h"

FileProcessor::FileProcessor(QObject* parent) : QObject(parent) {}

FileProcessor::~FileProcessor() {}

QString FileProcessor::readTxtFile(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open text file for reading:" << path << "-" << file.errorString();
        return QString();
    }
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    QString content = in.readAll();
    file.close();
    return content;
}

bool FileProcessor::saveTxtFile(const QString& path, const QString& content) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Failed to open text file for writing:" << path << "-" << file.errorString();
        return false;
    }
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << content;
    file.close();
    return true;
}

QString FileProcessor::readComFile(const QString& path) {
    ScriptRunner runner;
    return runner.convertComToTxt(path);
}
