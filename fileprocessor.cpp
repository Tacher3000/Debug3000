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
    QFileInfo fileInfo(path);
    QString fileName = fileInfo.fileName();
    QString currentDir = QCoreApplication::applicationDirPath();
    QString targetFilePath = currentDir + "/" + fileName;

    if (fileInfo.canonicalPath() != currentDir) {
        QFile sourceFile(path);
        if (!sourceFile.copy(targetFilePath)) {
            qDebug() << "Failed to copy COM file to current directory:" << path << "-" << sourceFile.errorString();
            return QString();
        }
    }

    QString disk = currentDir.left(1);
    QString exePath = currentDir + "/EXE";
    QString debugExe = exePath + "/DEBUG.exe";
    QString dosboxExe = exePath + "/DOSBoxX/dosbox-x.exe";
    QString dosboxConf = exePath + "/DOSBoxX/dosbox-x.conf";
    QString curPathDB = currentDir.mid(3);

    QString tempScriptPath = currentDir + "/u100.txt";
    QFile tempScript(tempScriptPath);
    if (!tempScript.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Failed to create temporary script file:" << tempScriptPath << "-" << tempScript.errorString();
        return QString();
    }
    QTextStream out(&tempScript);
    out << "n " << fileName << "\n";
    out << "l\n";
    out << "u 100 600\n";
    out << "q\n";
    tempScript.close();

    QStringList arguments;
    arguments << "-conf" << dosboxConf
              << "-c" << QString("mount d %1:\\").arg(disk)
              << "-c" << "d:"
              << "-c" << QString("cd %1").arg(curPathDB)
              << "-c" << QString("debug < u100.txt > temp000.txt")
              << "-c" << "exit";

    QProcess process;
    process.start(dosboxExe, arguments);
    if (!process.waitForFinished(30000)) {
        qDebug() << "DOSBox-X failed to complete:" << process.errorString();
        QFile::remove(tempScriptPath);
        QFile::remove(targetFilePath);
        return QString();
    }

    QString tempOutputPath = currentDir + "/temp000.txt";
    QFile tempOutput(tempOutputPath);
    if (!tempOutput.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to read temporary output file:" << tempOutputPath << "-" << tempOutput.errorString();
        QFile::remove(tempScriptPath);
        QFile::remove(targetFilePath);
        return QString();
    }

    QString codeOutputPath = currentDir + "/code.txt";
    QFile codeOutput(codeOutputPath);
    if (!codeOutput.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Failed to create code output file:" << codeOutputPath << "-" << codeOutput.errorString();
        tempOutput.close();
        QFile::remove(tempScriptPath);
        QFile::remove(targetFilePath);
        return QString();
    }

    QTextStream in(&tempOutput);
    QTextStream outCode(&codeOutput);
    bool start = false;
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.contains("-u 100 600")) {
            start = true;
            continue;
        }
        if (start) {
            QStringList tokens = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            if (tokens.size() >= 2) {
                QString instruction = tokens.mid(2).join(" ");
                if (!instruction.isEmpty()) {
                    outCode << instruction << "\n";
                }
            }
        }
    }
    tempOutput.close();
    codeOutput.close();

    if (!codeOutput.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to read code output file:" << codeOutputPath << "-" << codeOutput.errorString();
        QFile::remove(tempScriptPath);
        QFile::remove(tempOutputPath);
        QFile::remove(targetFilePath);
        return QString();
    }
    QTextStream codeIn(&codeOutput);
    QString content = codeIn.readAll();
    codeOutput.close();

    QFile::remove(tempScriptPath);
    QFile::remove(tempOutputPath);
    QFile::remove(codeOutputPath);
    if (fileInfo.canonicalPath() != currentDir) {
        QFile::remove(targetFilePath);
    }

    return content;
}
