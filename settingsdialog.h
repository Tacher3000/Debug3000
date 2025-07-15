#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QFontComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QColorDialog>

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    SettingsDialog(QWidget* parent = nullptr);
    QMap<QString, QVariant> getSettings() const;
    void setSettings(const QMap<QString, QVariant>& settings);
signals:
    void saveSettingsRequested(const QMap<QString, QVariant>& settings);
    void languageChanged(const QString& language);
private slots:
    void resetToDefaults();
    void selectBackgroundColor();
    void selectTextColor();
    void selectHighlightColor();
private:
    QComboBox* themeComboBox;
    QFontComboBox* fontComboBox;
    QSpinBox* fontSizeSpinBox;
    QCheckBox* standardLineNumberingCheckBox;
    QCheckBox* addressLineNumberingCheckBox;
    QCheckBox* lineWrapCheckBox;
    QCheckBox* autoSaveCheckBox;
    QSpinBox* autoSaveIntervalSpinBox;
    QCheckBox* autoCompleteCheckBox;
    QCheckBox* syntaxHighlightingCheckBox;
    QComboBox* languageComboBox;
    QPushButton* backgroundColorButton;
    QPushButton* textColorButton;
    QPushButton* highlightColorButton;
    QPushButton* resetButton;
    QPushButton* okButton;
    QPushButton* cancelButton;
    QColor backgroundColor;
    QColor textColor;
    QColor highlightColor;
};

#endif // SETTINGSDIALOG_H
