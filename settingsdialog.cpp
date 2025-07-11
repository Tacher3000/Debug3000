#include "settingsdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(tr("Настройки"));

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QLabel* themeLabel = new QLabel(tr("Тема интерфейса:"), this);
    themeComboBox = new QComboBox(this);
    themeComboBox->addItems({tr("Light"), tr("Dark")});
    mainLayout->addWidget(themeLabel);
    mainLayout->addWidget(themeComboBox);

    QLabel* fontLabel = new QLabel(tr("Шрифт:"), this);
    fontComboBox = new QFontComboBox(this);
    mainLayout->addWidget(fontLabel);
    mainLayout->addWidget(fontComboBox);

    QLabel* fontSizeLabel = new QLabel(tr("Размер шрифта:"), this);
    fontSizeSpinBox = new QSpinBox(this);
    fontSizeSpinBox->setRange(8, 24);
    mainLayout->addWidget(fontSizeLabel);
    mainLayout->addWidget(fontSizeSpinBox);

    standardLineNumberingCheckBox = new QCheckBox(tr("Показывать номера строк"), this);
    mainLayout->addWidget(standardLineNumberingCheckBox);

    addressLineNumberingCheckBox = new QCheckBox(tr("Показывать адреса команд"), this);
    mainLayout->addWidget(addressLineNumberingCheckBox);

    lineWrapCheckBox = new QCheckBox(tr("Перенос строк"), this);
    mainLayout->addWidget(lineWrapCheckBox);

    QLabel* encodingLabel = new QLabel(tr("Кодировка:"), this);
    encodingComboBox = new QComboBox(this);
    encodingComboBox->addItems({tr("UTF-8"), tr("Windows-1251"), tr("ASCII")});
    mainLayout->addWidget(encodingLabel);
    mainLayout->addWidget(encodingComboBox);

    QLabel* tabSizeLabel = new QLabel(tr("Размер табуляции:"), this);
    tabSizeSpinBox = new QSpinBox(this);
    tabSizeSpinBox->setRange(2, 8);
    mainLayout->addWidget(tabSizeLabel);
    mainLayout->addWidget(tabSizeSpinBox);

    resetButton = new QPushButton(tr("Восстановить по умолчанию"), this);
    mainLayout->addWidget(resetButton);
    connect(resetButton, &QPushButton::clicked, this, &SettingsDialog::resetToDefaults);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
    okButton = buttonBox->addButton(tr("OK"), QDialogButtonBox::AcceptRole);
    okButton->setIcon(style()->standardIcon(QStyle::SP_DialogOkButton));
    cancelButton = buttonBox->addButton(tr("Отмена"), QDialogButtonBox::RejectRole);
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
    QFont font = fontComboBox->currentFont();
    font.setPointSize(fontSizeSpinBox->value());
    settings["font"] = font;
    settings["standardLineNumbering"] = standardLineNumberingCheckBox->isChecked();
    settings["addressLineNumbering"] = addressLineNumberingCheckBox->isChecked();
    settings["lineWrap"] = lineWrapCheckBox->isChecked();
    settings["encoding"] = encodingComboBox->currentText();
    settings["tabSize"] = tabSizeSpinBox->value();
    return settings;
}

void SettingsDialog::setSettings(const QMap<QString, QVariant>& settings) {
    themeComboBox->setCurrentText(settings["theme"].toString());
    QFont font = settings["font"].value<QFont>();
    fontComboBox->setCurrentFont(font);
    fontSizeSpinBox->setValue(font.pointSize());
    standardLineNumberingCheckBox->setChecked(settings["standardLineNumbering"].toBool());
    addressLineNumberingCheckBox->setChecked(settings["addressLineNumbering"].toBool());
    lineWrapCheckBox->setChecked(settings["lineWrap"].toBool());
    encodingComboBox->setCurrentText(settings["encoding"].toString());
    tabSizeSpinBox->setValue(settings["tabSize"].toInt());
}

void SettingsDialog::resetToDefaults() {
    themeComboBox->setCurrentText(tr("Light"));
    QFont font("Courier New", 10);
    fontComboBox->setCurrentFont(font);
    fontSizeSpinBox->setValue(10);
    standardLineNumberingCheckBox->setChecked(true);
    addressLineNumberingCheckBox->setChecked(true);
    lineWrapCheckBox->setChecked(false);
    encodingComboBox->setCurrentText(tr("UTF-8"));
    tabSizeSpinBox->setValue(4);
}
