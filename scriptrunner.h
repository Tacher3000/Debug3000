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
    QString runDebugScript(const QString& filePath);
};

#endif // SCRIPTRUNNER_H
