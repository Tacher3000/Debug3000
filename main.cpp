#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include <QProcess>
#include "settingsmanager.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    SettingsManager settingsManager;
    QMap<QString, QVariant> settings = settingsManager.loadSettings();
    QString language = settings["language"].toString();
    QTranslator translator;
    QString langCode = (language == "Russian") ? "ru" : "en";
    if (translator.load(QString(":/translations/DebugCrafter_%1.qm").arg(langCode))) {
        a.installTranslator(&translator);
    }

    MainWindow w;
    w.setMinimumSize(800, 600);
    w.showMaximized();
    return a.exec();
}
