#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QSettings>
#include <QMap>
#include <QVariant>

class SettingsManager : public QObject {
    Q_OBJECT
public:
    SettingsManager();
    ~SettingsManager();
    QMap<QString, QVariant> loadSettings();
    void saveSettings(const QMap<QString, QVariant>& settings);
    void applySettings();
    void resetToDefaults();
signals:
    void settingsChanged(const QMap<QString, QVariant>& settings);
private:
    QSettings* settings;
};

#endif // SETTINGSMANAGER_H
