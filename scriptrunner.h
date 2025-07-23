#ifndef SCRIPTRUNNER_H
#define SCRIPTRUNNER_H

#include <QObject>
#include <QString>

class ScriptRunner : public QObject {
    Q_OBJECT
public:
    ScriptRunner(QObject* parent = nullptr);
    ~ScriptRunner();
    QString convertComToTxt(const QString& path);
    QString pasteCodeToDebug(const QString& filePath);
    void compileAndRunCom(const QString& filePath);
signals:
    void compileAndRunFinished(const QString& output);
};

#endif // SCRIPTRUNNER_H
