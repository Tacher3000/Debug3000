#include "settingsdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QRegularExpressionValidator>
#include <QColorDialog>

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(tr("Settings"));

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QLabel* themeLabel = new QLabel(tr("Theme:"), this);
    themeComboBox = new QComboBox(this);
    themeComboBox->addItem(tr("Light"), "Light");
    themeComboBox->addItem(tr("Dark"), "Dark");
    themeComboBox->addItem(tr("Custom"), "Custom");
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

    syntaxHighlightingCheckBox = new QCheckBox(tr("Enable Syntax Highlighting"), this);
    mainLayout->addWidget(syntaxHighlightingCheckBox);

    QLabel* languageLabel = new QLabel(tr("Language:"), this);
    languageComboBox = new QComboBox(this);
    languageComboBox->addItem(tr("English"), "English");
    languageComboBox->addItem(tr("Russian"), "Russian");
    mainLayout->addWidget(languageLabel);
    mainLayout->addWidget(languageComboBox);
    connect(languageComboBox, &QComboBox::currentTextChanged, this, [this](const QString&) {
        emit languageChanged(languageComboBox->itemData(languageComboBox->currentIndex()).toString());
    });

    showMemoryDumpCheckBox = new QCheckBox(tr("Show Memory Dump"), this);
    mainLayout->addWidget(showMemoryDumpCheckBox);

    QLabel* memoryDumpSegmentLabel = new QLabel(tr("Memory Dump Segment:"), this);
    memoryDumpSegmentEdit = new QLineEdit(this);
    memoryDumpSegmentEdit->setPlaceholderText("1000");
    memoryDumpSegmentEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9A-Fa-f]{1,4}"), memoryDumpSegmentEdit));
    mainLayout->addWidget(memoryDumpSegmentLabel);
    mainLayout->addWidget(memoryDumpSegmentEdit);

    QLabel* memoryDumpOffsetLabel = new QLabel(tr("Memory Dump Offset:"), this);
    memoryDumpOffsetEdit = new QLineEdit(this);
    memoryDumpOffsetEdit->setPlaceholderText("200");
    memoryDumpOffsetEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9A-Fa-f]{1,4}"), memoryDumpOffsetEdit));
    mainLayout->addWidget(memoryDumpOffsetLabel);
    mainLayout->addWidget(memoryDumpOffsetEdit);

    QLabel* memoryDumpLineCountLabel = new QLabel(tr("Memory Dump Line Count:"), this);
    memoryDumpLineCountSpinBox = new QSpinBox(this);
    memoryDumpLineCountSpinBox->setRange(1, 32);
    memoryDumpLineCountSpinBox->setValue(8);
    mainLayout->addWidget(memoryDumpLineCountLabel);
    mainLayout->addWidget(memoryDumpLineCountSpinBox);

    showOutputConsoleCheckBox = new QCheckBox(tr("Show Output Console"), this);
    mainLayout->addWidget(showOutputConsoleCheckBox);

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
    settings["theme"] = themeComboBox->itemData(themeComboBox->currentIndex()).toString();
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
    settings["syntaxHighlighting"] = syntaxHighlightingCheckBox->isChecked();
    settings["language"] = languageComboBox->itemData(languageComboBox->currentIndex()).toString();
    settings["showMemoryDump"] = showMemoryDumpCheckBox->isChecked();
    settings["memoryDumpSegment"] = memoryDumpSegmentEdit->text();
    settings["memoryDumpOffset"] = memoryDumpOffsetEdit->text();
    settings["memoryDumpLineCount"] = memoryDumpLineCountSpinBox->value();
    settings["showOutputConsole"] = showOutputConsoleCheckBox->isChecked();
    return settings;
}

void SettingsDialog::setSettings(const QMap<QString, QVariant>& settings) {
    int themeIndex = themeComboBox->findData(settings["theme"].toString());
    themeComboBox->setCurrentIndex(themeIndex != -1 ? themeIndex : 0);

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
    syntaxHighlightingCheckBox->setChecked(settings["syntaxHighlighting"].toBool());

    int languageIndex = languageComboBox->findData(settings["language"].toString());
    languageComboBox->setCurrentIndex(languageIndex != -1 ? languageIndex : 0);

    showMemoryDumpCheckBox->setChecked(settings["showMemoryDump"].toBool());
    memoryDumpSegmentEdit->setText(settings["memoryDumpSegment"].toString());
    memoryDumpOffsetEdit->setText(settings["memoryDumpOffset"].toString());
    memoryDumpLineCountSpinBox->setValue(settings["memoryDumpLineCount"].toInt());
    showOutputConsoleCheckBox->setChecked(settings["showOutputConsole"].toBool());
}

void SettingsDialog::resetToDefaults() {
    themeComboBox->setCurrentIndex(themeComboBox->findData("Light"));
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
    syntaxHighlightingCheckBox->setChecked(true);
    languageComboBox->setCurrentIndex(languageComboBox->findData("English"));
    showMemoryDumpCheckBox->setChecked(false);
    memoryDumpSegmentEdit->setText("1000");
    memoryDumpOffsetEdit->setText("200");
    memoryDumpLineCountSpinBox->setValue(8);
    showOutputConsoleCheckBox->setChecked(false);
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
