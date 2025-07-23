#include "scriptrunner.h"
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QDir>
#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTimer>
#include <windows.h>
#include <QClipboard>
#include <QApplication>

ScriptRunner::ScriptRunner(QObject* parent) : QObject(parent) {}

ScriptRunner::~ScriptRunner() {}

QString ScriptRunner::convertComToTxt(const QString& path) {
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

QString ScriptRunner::pasteCodeToDebug(const QString& filePath) {
    QFile inputFile(filePath);
    if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Не удалось открыть входной файл:" << filePath << "-" << inputFile.errorString();
        return QString();
    }
    QTextStream in(&inputFile);
    in.setEncoding(QStringConverter::Utf8);
    QString content = in.readAll();
    inputFile.close();

    QString currentDir = QCoreApplication::applicationDirPath();
    QString disk = currentDir.left(1);
    QString exePath = currentDir + "/EXE";
    QString dosboxExe = exePath + "/DOSBoxX/dosbox-x.exe";
    QString dosboxConf = exePath + "/DOSBoxX/dosbox-x.conf";
    QString curPathDB = currentDir.mid(3);

    LoadKeyboardLayout(L"00000409", KLF_ACTIVATE);

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(content);

    QStringList arguments;
    arguments << "-conf" << dosboxConf
              << "-c" << QString("mount d %1:\\").arg(disk)
              << "-c" << "d:"
              << "-c" << QString("cd %1").arg(curPathDB)
              << "-c" << "debug";

    QProcess::startDetached(dosboxExe, arguments);

    QTimer::singleShot(2000, this, [this]() {
        INPUT inputs[4] = {};
        ZeroMemory(inputs, sizeof(inputs));

        inputs[0].type = INPUT_KEYBOARD;
        inputs[0].ki.wVk = VK_CONTROL;

        inputs[1].type = INPUT_KEYBOARD;
        inputs[1].ki.wVk = VK_F6;

        inputs[2].type = INPUT_KEYBOARD;
        inputs[2].ki.wVk = VK_F6;
        inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;

        inputs[3].type = INPUT_KEYBOARD;
        inputs[3].ki.wVk = VK_CONTROL;
        inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

        SendInput(4, inputs, sizeof(INPUT));
    });

    return QString();
}

void ScriptRunner::compileAndRunCom(const QString& filePath) {
    QFile inputFile(filePath);
    if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Не удалось открыть входной файл:" << filePath << "-" << inputFile.errorString();
        emit compileAndRunFinished(QString());
        return;
    }
    QTextStream in(&inputFile);
    in.setEncoding(QStringConverter::Utf8);
    QString content = in.readAll();
    inputFile.close();

    // bool hasWriteCommand = content.contains(QRegularExpression("^\\s*w\\s*$", QRegularExpression::MultilineOption));

    // if (!hasWriteCommand) {
    //     qDebug() << "Команда 'w' не найдена в скрипте. Компиляция COM-файла невозможна.";
    //     emit compileAndRunFinished(QString());
    //     return;
    // }

    QString currentDir = QCoreApplication::applicationDirPath();
    QString tempScriptPath = currentDir + "/run.txt";
    QFile::remove(tempScriptPath);
    QFile tempScript(tempScriptPath);
    if (!tempScript.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Не удалось создать временный скрипт:" << tempScriptPath << "-" << tempScript.errorString();
        emit compileAndRunFinished(QString());
        return;
    }
    QTextStream scriptOut(&tempScript);
    scriptOut.setEncoding(QStringConverter::Utf8);
    scriptOut << content;
    tempScript.close();

    QString disk = currentDir.left(1);
    QString exePath = currentDir + "/EXE";
    QString dosboxExe = exePath + "/DOSBoxX/dosbox-x.exe";
    QString dosboxConf = exePath + "/DOSBoxX/dosbox-x.conf";
    QString curPathDB = currentDir.mid(3);
    QString tempOutputPath = currentDir + "/out.txt";
    QString comFilePath = currentDir + "/out.com";

    QFile::remove(tempOutputPath);
    QFile::remove(comFilePath);

    QStringList arguments;
    arguments << "-conf" << dosboxConf
              << "-c" << QString("mount d %1:\\").arg(disk)
              << "-c" << "d:"
              << "-c" << QString("cd %1").arg(curPathDB)
              << "-c" << QString("debug < run.txt > out.txt")
              << "-c" << "if exist out.com out.com"
              << "-c" << "exit";

    QProcess* process = new QProcess(this);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=](int exitCode, QProcess::ExitStatus exitStatus) {
        QString outputContent;
        QFile outputFile(tempOutputPath);
        if (outputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream outIn(&outputFile);
            outIn.setEncoding(QStringConverter::Utf8);
            outputContent = outIn.readAll();
            outputFile.close();
            qDebug() << "Output file read successfully:" << tempOutputPath;
            QFile::remove(tempOutputPath);
        } else {
            qDebug() << "Не удалось открыть файл вывода:" << tempOutputPath << "-" << outputFile.errorString();
        }

        if (QFile::exists(tempScriptPath)) {
            QFile::remove(tempScriptPath);
        }
        if (QFile::exists(comFilePath)) {
            QFile::remove(comFilePath);
        }

        emit compileAndRunFinished(outputContent);
        process->deleteLater();
    });

    process->start(dosboxExe, arguments);
    if (!process->waitForStarted()) {
        qDebug() << "Не удалось запустить DOSBox-X:" << process->errorString();
        if (QFile::exists(tempScriptPath)) {
            QFile::remove(tempScriptPath);
        }
        emit compileAndRunFinished(QString());
        process->deleteLater();
    }
}
