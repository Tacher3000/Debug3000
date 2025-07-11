#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QSettings>
#include <QMap>
#include <QVariant>

class SettingsManager : public QObject {
    Q_OBJECT
public:
    SettingsManager(QObject* parent = nullptr);
    ~SettingsManager();
    QMap<QString, QVariant> loadSettings();
    void saveSettings(const QMap<QString, QVariant>& settings);
    void applySettings();
    void resetToDefaults();
signals:
    void settingsChanged(const QMap<QString, QVariant>& settings);
private:
    static const QString ORGANIZATION_NAME;
    static const QString APPLICATION_NAME;
    QSettings* settings;
};

#endif // SETTINGSMANAGER_H
