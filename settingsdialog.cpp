#include "settingsdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(tr("Settings"));

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QLabel* themeLabel = new QLabel(tr("Theme:"), this);
    themeComboBox = new QComboBox(this);
    themeComboBox->addItems({tr("Light"), tr("Dark"), tr("Custom")});
    mainLayout->addWidget(themeLabel);
    mainLayout->addWidget(themeComboBox);

    QLabel* backgroundColorLabel = new QLabel(tr("Background Color:"), this);
    backgroundColorButton = new QPushButton(tr("Select Color"), this);
    connect(backgroundColorButton, &QPushButton::clicked, this, &SettingsDialog::selectBackgroundColor);
    mainLayout->addWidget(backgroundColorLabel);
    mainLayout->addWidget(backgroundColorButton);

    QLabel* textColorLabel = new QLabel(tr("Text Color:"), this);
    textColorButton = new QPushButton(tr("Select Color"), this);
    connect(textColorButton, &QPushButton::clicked, this, &SettingsDialog::selectTextColor);
    mainLayout->addWidget(textColorLabel);
    mainLayout->addWidget(textColorButton);

    QLabel* highlightColorLabel = new QLabel(tr("Highlight Color:"), this);
    highlightColorButton = new QPushButton(tr("Select Color"), this);
    connect(highlightColorButton, &QPushButton::clicked, this, &SettingsDialog::selectHighlightColor);
    mainLayout->addWidget(highlightColorLabel);
    mainLayout->addWidget(highlightColorButton);

    QLabel* fontLabel = new QLabel(tr("Font:"), this);
    fontComboBox = new QFontComboBox(this);
    mainLayout->addWidget(fontLabel);
    mainLayout->addWidget(fontComboBox);

    QLabel* fontSizeLabel = new QLabel(tr("Font Size:"), this);
    fontSizeSpinBox = new QSpinBox(this);
    fontSizeSpinBox->setRange(8, 24);
    mainLayout->addWidget(fontSizeLabel);
    mainLayout->addWidget(fontSizeSpinBox);

    standardLineNumberingCheckBox = new QCheckBox(tr("Show Line Numbers"), this);
    mainLayout->addWidget(standardLineNumberingCheckBox);

    addressLineNumberingCheckBox = new QCheckBox(tr("Show Instruction Addresses"), this);
    mainLayout->addWidget(addressLineNumberingCheckBox);

    lineWrapCheckBox = new QCheckBox(tr("Line Wrap"), this);
    mainLayout->addWidget(lineWrapCheckBox);

    autoSaveCheckBox = new QCheckBox(tr("Enable Auto-Save"), this);
    mainLayout->addWidget(autoSaveCheckBox);

    QLabel* autoSaveIntervalLabel = new QLabel(tr("Auto-Save Interval (minutes):"), this);
    autoSaveIntervalSpinBox = new QSpinBox(this);
    autoSaveIntervalSpinBox->setRange(1, 10);
    mainLayout->addWidget(autoSaveIntervalLabel);
    mainLayout->addWidget(autoSaveIntervalSpinBox);

    autoCompleteCheckBox = new QCheckBox(tr("Enable Auto-Completion"), this);
    mainLayout->addWidget(autoCompleteCheckBox);

    syntaxHighlightingCheckBox = new QCheckBox(tr("Enable Syntax Highlighting"), this);
    mainLayout->addWidget(syntaxHighlightingCheckBox);

    QLabel* languageLabel = new QLabel(tr("Language:"), this);
    languageComboBox = new QComboBox(this);
    languageComboBox->addItems({tr("English"), tr("Russian")});
    mainLayout->addWidget(languageLabel);
    mainLayout->addWidget(languageComboBox);
    connect(languageComboBox, &QComboBox::currentTextChanged, this, &SettingsDialog::languageChanged);

    resetButton = new QPushButton(tr("Reset to Defaults"), this);
    mainLayout->addWidget(resetButton);
    connect(resetButton, &QPushButton::clicked, this, &SettingsDialog::resetToDefaults);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
    okButton = buttonBox->addButton(tr("OK"), QDialogButtonBox::AcceptRole);
    okButton->setIcon(style()->standardIcon(QStyle::SP_DialogOkButton));
    cancelButton = buttonBox->addButton(tr("Cancel"), QDialogButtonBox::RejectRole);
    cancelButton->setIcon(style()->standardIcon(QStyle::SP_DialogCancelButton));
    mainLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        emit saveSettingsRequested(getSettings());
        accept();
    });
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

QMap<QString, QVariant> SettingsDialog::getSettings() const {
    QMap<QString, QVariant> settings;
    settings["theme"] = themeComboBox->currentText();
    settings["backgroundColor"] = backgroundColor;
    settings["textColor"] = textColor;
    settings["highlightColor"] = highlightColor;
    QFont font = fontComboBox->currentFont();
    font.setPointSize(fontSizeSpinBox->value());
    settings["font"] = font;
    settings["standardLineNumbering"] = standardLineNumberingCheckBox->isChecked();
    settings["addressLineNumbering"] = addressLineNumberingCheckBox->isChecked();
    settings["lineWrap"] = lineWrapCheckBox->isChecked();
    settings["autoSave"] = autoSaveCheckBox->isChecked();
    settings["autoSaveInterval"] = autoSaveIntervalSpinBox->value();
    settings["autoComplete"] = autoCompleteCheckBox->isChecked();
    settings["syntaxHighlighting"] = syntaxHighlightingCheckBox->isChecked();
    settings["language"] = languageComboBox->currentText();
    return settings;
}

void SettingsDialog::setSettings(const QMap<QString, QVariant>& settings) {
    themeComboBox->setCurrentText(settings["theme"].toString());
    backgroundColor = settings["backgroundColor"].value<QColor>();
    textColor = settings["textColor"].value<QColor>();
    highlightColor = settings["highlightColor"].value<QColor>();
    QFont font = settings["font"].value<QFont>();
    fontComboBox->setCurrentFont(font);
    fontSizeSpinBox->setValue(font.pointSize());
    standardLineNumberingCheckBox->setChecked(settings["standardLineNumbering"].toBool());
    addressLineNumberingCheckBox->setChecked(settings["addressLineNumbering"].toBool());
    lineWrapCheckBox->setChecked(settings["lineWrap"].toBool());
    autoSaveCheckBox->setChecked(settings["autoSave"].toBool());
    autoSaveIntervalSpinBox->setValue(settings["autoSaveInterval"].toInt());
    autoCompleteCheckBox->setChecked(settings["autoComplete"].toBool());
    syntaxHighlightingCheckBox->setChecked(settings["syntaxHighlighting"].toBool());
    languageComboBox->setCurrentText(settings["language"].toString());
}

void SettingsDialog::resetToDefaults() {
    themeComboBox->setCurrentText(tr("Light"));
    backgroundColor = Qt::white;
    textColor = Qt::black;
    highlightColor = QColor(Qt::darkGray).lighter(160);
    QFont font("Courier New", 10);
    fontComboBox->setCurrentFont(font);
    fontSizeSpinBox->setValue(10);
    standardLineNumberingCheckBox->setChecked(true);
    addressLineNumberingCheckBox->setChecked(true);
    lineWrapCheckBox->setChecked(false);
    autoSaveCheckBox->setChecked(false);
    autoSaveIntervalSpinBox->setValue(5);
    autoCompleteCheckBox->setChecked(true);
    syntaxHighlightingCheckBox->setChecked(true);
    languageComboBox->setCurrentText(tr("English"));
}

void SettingsDialog::selectBackgroundColor() {
    QColor color = QColorDialog::getColor(backgroundColor, this, tr("Select Background Color"));
    if (color.isValid()) {
        backgroundColor = color;
    }
}

void SettingsDialog::selectTextColor() {
    QColor color = QColorDialog::getColor(textColor, this, tr("Select Text Color"));
    if (color.isValid()) {
        textColor = color;
    }
}

void SettingsDialog::selectHighlightColor() {
    QColor color = QColorDialog::getColor(highlightColor, this, tr("Select Highlight Color"));
    if (color.isValid()) {
        highlightColor = color;
    }
}
