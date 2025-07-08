#ifndef SCRIPTRUNNER_H
#define SCRIPTRUNNER_H

#include <QString>

class ScriptRunner {
public:
    ScriptRunner();
    ~ScriptRunner();
    QString convertComToTxt(const QString& path);
    QString runDebugScript(const QString& filePath);
};

#endif // SCRIPTRUNNER_H
