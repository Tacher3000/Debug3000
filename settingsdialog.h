#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QFontComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    SettingsDialog(QWidget* parent = nullptr);
    QMap<QString, QVariant> getSettings() const;
    void setSettings(const QMap<QString, QVariant>& settings);
signals:
    void saveSettingsRequested(const QMap<QString, QVariant>& settings);
private slots:
    void resetToDefaults();
private:
    QComboBox* themeComboBox;
    QFontComboBox* fontComboBox;
    QSpinBox* fontSizeSpinBox;
    QCheckBox* standardLineNumberingCheckBox;
    QCheckBox* addressLineNumberingCheckBox;
    QCheckBox* lineWrapCheckBox;
    QComboBox* encodingComboBox;
    QSpinBox* tabSizeSpinBox;
    QPushButton* resetButton;
    QPushButton* okButton;
    QPushButton* cancelButton;
};

#endif // SETTINGSDIALOG_H
