#include "scriptrunner.h"
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QDir>
#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <QRegularExpression>

ScriptRunner::ScriptRunner(QObject* parent) : QObject(parent) {}

ScriptRunner::~ScriptRunner() {}

QString ScriptRunner::convertComToTxt(const QString& path) { return QString(); }

QString ScriptRunner::runDebugScript(const QString& filePath) {
    QString currentDir = QCoreApplication::applicationDirPath();

    QFile inputFile(filePath);
    if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Не удалось открыть входной файл:" << filePath << "-" << inputFile.errorString();
        return QString();
    }
    QTextStream in(&inputFile);
    in.setEncoding(QStringConverter::Utf8);
    QString content = in.readAll();
    inputFile.close();

    QString tempScriptPath = currentDir + "/run.txt";
    QFile tempScript(tempScriptPath);
    if (!tempScript.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Не удалось создать временный скрипт:" << tempScriptPath << "-" << tempScript.errorString();
        return QString();
    }
    QTextStream scriptOut(&tempScript);
    scriptOut << content;
    tempScript.close();

    QString disk = currentDir.left(1);
    QString exePath = currentDir + "/EXE";
    QString dosboxExe = exePath + "/DOSBoxX/dosbox-x.exe";
    QString dosboxConf = exePath + "/DOSBoxX/dosbox-x.conf";
    QString curPathDB = currentDir.mid(3);

    QString tempOutputPath = currentDir + "/out.txt";
    QString comFilePath = currentDir + "/out.com";

    QStringList arguments;
    arguments << "-conf" << dosboxConf
              << "-c" << QString("mount d %1:\\").arg(disk)
              << "-c" << "d:"
              << "-c" << QString("cd %1").arg(curPathDB)
              << "-c" << QString("debug < run.txt > out.txt")
              << "-c" << QString("out.com");

    QProcess* process = new QProcess(this);
    process->start(dosboxExe, arguments);
    if (!process->waitForStarted()) {
        qDebug() << "Не удалось запустить DOSBox-X:" << process->errorString();
        QFile::remove(tempScriptPath);
        delete process;
        return QString();
    }

    connect(process, &QProcess::finished, this, [=]() {
        QFile::remove(tempScriptPath);
        QFile::remove(tempOutputPath);
        QFile::remove(comFilePath);
        delete process;
    });

    return QString();
}
