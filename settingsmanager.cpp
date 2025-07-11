#include "settingsmanager.h"
#include <QFont>

const QString SettingsManager::ORGANIZATION_NAME = "Tacher3000";
const QString SettingsManager::APPLICATION_NAME = "CodeEditor";

SettingsManager::SettingsManager(QObject* parent) : QObject(parent) {
    settings = new QSettings(ORGANIZATION_NAME, APPLICATION_NAME, this);
}

SettingsManager::~SettingsManager() {
}

QMap<QString, QVariant> SettingsManager::loadSettings() {
    QMap<QString, QVariant> result;
    result["theme"] = settings->value("theme", "Light").toString();
    result["font"] = settings->value("font", QFont("Courier New", 10));
    result["standardLineNumbering"] = settings->value("standardLineNumbering", true).toBool();
    result["addressLineNumbering"] = settings->value("addressLineNumbering", true).toBool();
    result["lineWrap"] = settings->value("lineWrap", false).toBool();
    result["encoding"] = settings->value("encoding", "UTF-8").toString();
    result["tabSize"] = settings->value("tabSize", 4).toInt();
    return result;
}

void SettingsManager::saveSettings(const QMap<QString, QVariant>& settingsMap) {
    for (auto it = settingsMap.constBegin(); it != settingsMap.constEnd(); ++it) {
        settings->setValue(it.key(), it.value());
    }
    settings->sync();
    emit settingsChanged(settingsMap);
}

void SettingsManager::applySettings() {
    emit settingsChanged(loadSettings());
}

void SettingsManager::resetToDefaults() {
    QMap<QString, QVariant> defaultSettings;
    defaultSettings["theme"] = "Light";
    defaultSettings["font"] = QFont("Courier New", 10);
    defaultSettings["standardLineNumbering"] = true;
    defaultSettings["addressLineNumbering"] = true;
    defaultSettings["lineWrap"] = false;
    defaultSettings["encoding"] = "UTF-8";
    defaultSettings["tabSize"] = 4;
    saveSettings(defaultSettings);
}
