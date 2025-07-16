#include "settingsmanager.h"
#include <QFont>
#include <QColor>

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
    result["backgroundColor"] = settings->value("backgroundColor", QColor(Qt::white)).value<QColor>();
    result["textColor"] = settings->value("textColor", QColor(Qt::black)).value<QColor>();
    result["highlightColor"] = settings->value("highlightColor", QColor(Qt::darkGray).lighter(160)).value<QColor>();
    result["font"] = settings->value("font", QFont("Courier New", 10));
    result["standardLineNumbering"] = settings->value("standardLineNumbering", true).toBool();
    result["addressLineNumbering"] = settings->value("addressLineNumbering", true).toBool();
    result["lineWrap"] = settings->value("lineWrap", false).toBool();
    result["autoSave"] = settings->value("autoSave", false).toBool();
    result["syntaxHighlighting"] = settings->value("syntaxHighlighting", true).toBool();
    result["language"] = settings->value("language", "English").toString();
    result["showMemoryDump"] = settings->value("showMemoryDump", false).toBool();
    result["memoryDumpSegment"] = settings->value("memoryDumpSegment", "1000").toString();
    result["memoryDumpOffset"] = settings->value("memoryDumpOffset", "200").toString();
    result["memoryDumpLineCount"] = settings->value("memoryDumpLineCount", 8).toInt();
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
    defaultSettings["backgroundColor"] = QColor(Qt::white);
    defaultSettings["textColor"] = QColor(Qt::black);
    defaultSettings["highlightColor"] = QColor(Qt::darkGray).lighter(160);
    defaultSettings["font"] = QFont("Courier New", 10);
    defaultSettings["standardLineNumbering"] = true;
    defaultSettings["addressLineNumbering"] = true;
    defaultSettings["lineWrap"] = false;
    defaultSettings["autoSave"] = false;
    defaultSettings["syntaxHighlighting"] = true;
    defaultSettings["language"] = "English";
    defaultSettings["showMemoryDump"] = false;
    defaultSettings["memoryDumpSegment"] = "1000";
    defaultSettings["memoryDumpOffset"] = "200";
    defaultSettings["memoryDumpLineCount"] = 8;
    saveSettings(defaultSettings);
}
