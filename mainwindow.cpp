#include "mainwindow.h"
#include <QMenuBar>
#include <QToolBar>
#include <QAction>
#include <QFileDialog>
#include <QMap>
#include <QFileInfo>
#include <QApplication>
#include <QProcess>
#include <QMessageBox>
#include <QCloseEvent>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle(tr("DebugCrafter"));
    tabWidget = new QTabWidget(this);
    tabWidget->setTabsClosable(true);
    setCentralWidget(tabWidget);
    settingsManager = new SettingsManager(this);
    fileController = new FileController(this);
    createMenus();
    createToolBar();
    connect(settingsManager, &SettingsManager::settingsChanged, this, &MainWindow::updateEditors);
    connect(tabWidget, &QTabWidget::tabCloseRequested, this, [this](int index) { tabWidget->removeTab(index); });
    settingsManager->applySettings();
}

MainWindow::~MainWindow() {
}

void MainWindow::closeEvent(QCloseEvent* event) {
    bool hasUnsavedChanges = false;
    for (int i = 0; i < tabWidget->count(); ++i) {
        CodeEditor* editor = qobject_cast<CodeEditor*>(tabWidget->widget(i));
        if (editor && editor->document()->isModified()) {
            hasUnsavedChanges = true;
            break;
        }
    }

    if (hasUnsavedChanges) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            tr("Unsaved Changes"),
            tr("You have unsaved changes. Would you like to save them before exiting?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
            );

        if (reply == QMessageBox::Save) {
            for (int i = 0; i < tabWidget->count(); ++i) {
                CodeEditor* editor = qobject_cast<CodeEditor*>(tabWidget->widget(i));
                if (editor && editor->document()->isModified()) {
                    tabWidget->setCurrentIndex(i);
                    saveFile();
                }
            }
        } else if (reply == QMessageBox::Cancel) {
            event->ignore();
            return;
        }
    }

    event->accept();
}

void MainWindow::createMenus() {
    fileMenu = menuBar()->addMenu(tr("File"));
    newAction = fileMenu->addAction(tr("New"));
    openAction = fileMenu->addAction(tr("Open"));
    saveAction = fileMenu->addAction(tr("Save"));
    saveAsAction = fileMenu->addAction(tr("Save As"));
    runAction = fileMenu->addAction(tr("Run"));
    settingsMenu = menuBar()->addMenu(tr("Settings"));
    settingsAction = settingsMenu->addAction(tr("Preferences"));
    connect(newAction, &QAction::triggered, this, &MainWindow::newFile);
    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveFile);
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveFileAs);
    connect(runAction, &QAction::triggered, this, &MainWindow::runCode);
    connect(settingsAction, &QAction::triggered, this, &MainWindow::showSettingsDialog);
}

void MainWindow::createToolBar() {
    toolBar = addToolBar(tr("Tools"));
    toolBar->addAction(newAction);
    toolBar->addAction(openAction);
    toolBar->addAction(saveAction);
    toolBar->addAction(saveAsAction);
    toolBar->addAction(runAction);
    toolBar->addAction(settingsAction);
}

void MainWindow::updateInterfaceTranslations() {
    setWindowTitle(tr("DebugCrafter"));
    fileMenu->setTitle(tr("File"));
    newAction->setText(tr("New"));
    openAction->setText(tr("Open"));
    saveAction->setText(tr("Save"));
    saveAsAction->setText(tr("Save As"));
    runAction->setText(tr("Run"));
    settingsMenu->setTitle(tr("Settings"));
    settingsAction->setText(tr("Preferences"));
    toolBar->setWindowTitle(tr("Tools"));
    for (int i = 0; i < tabWidget->count(); ++i) {
        if (tabWidget->tabText(i) == "New File" || tabWidget->tabText(i) == tr("New File")) {
            tabWidget->setTabText(i, tr("New File"));
        }
    }
}

CodeEditor* MainWindow::getCurrentEditor() const {
    return qobject_cast<CodeEditor*>(tabWidget->currentWidget());
}

void MainWindow::newFile() {
    CodeEditor* editor = new CodeEditor();
    tabWidget->addTab(editor, tr("New File"));
    tabWidget->setCurrentWidget(editor);
    QMap<QString, QVariant> settings = settingsManager->loadSettings();
    updateEditors(settings);
    if (settings["autoSave"].toBool()) {
        connect(editor, &QPlainTextEdit::textChanged, this, &MainWindow::handleTextChanged);
    }
}

void MainWindow::openFile() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("Text and COM Files (*.txt *.com *.COM);;All Files (*)"));
    if (!fileName.isEmpty()) {
        QString content = fileController->openFile(fileName);
        CodeEditor* editor = new CodeEditor();
        editor->setText(content);
        tabWidget->addTab(editor, QFileInfo(fileName).fileName());
        tabWidget->setCurrentWidget(editor);
        QMap<QString, QVariant> settings = settingsManager->loadSettings();
        updateEditors(settings);
        tabWidget->setTabToolTip(tabWidget->currentIndex(), fileName);
        if (settings["autoSave"].toBool()) {
            connect(editor, &QPlainTextEdit::textChanged, this, &MainWindow::handleTextChanged);
        }
    }
}

void MainWindow::saveFile() {
    CodeEditor* editor = getCurrentEditor();
    if (!editor) return;

    QString fileName = tabWidget->tabToolTip(tabWidget->currentIndex());
    if (fileName.isEmpty() || tabWidget->tabText(tabWidget->currentIndex()) == tr("New File") || fileName.endsWith(".com", Qt::CaseInsensitive)) {
        saveFileAs();
        return;
    }

    if (fileController->saveFile(fileName, editor->getText())) {
        tabWidget->setTabText(tabWidget->currentIndex(), QFileInfo(fileName).fileName());
        editor->document()->setModified(false);
    }
}

void MainWindow::saveFileAs() {
    CodeEditor* editor = getCurrentEditor();
    if (!editor) return;

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File As"), "", tr("Text Files (*.txt);;All Files (*)"));
    if (!fileName.isEmpty()) {
        if (fileController->saveAsFile(fileName, editor->getText())) {
            tabWidget->setTabText(tabWidget->currentIndex(), QFileInfo(fileName).fileName());
            tabWidget->setTabToolTip(tabWidget->currentIndex(), fileName);
            editor->document()->setModified(false);
        }
    }
}

void MainWindow::runCode() {
    CodeEditor* editor = getCurrentEditor();
    if (!editor) return;

    QString fileName = tabWidget->tabToolTip(tabWidget->currentIndex());
    bool isComFile = fileName.endsWith(".com", Qt::CaseInsensitive);

    if (fileName.isEmpty() || tabWidget->tabText(tabWidget->currentIndex()) == tr("New File")) {
        fileName = QCoreApplication::applicationDirPath() + (isComFile ? "/temp_run.com" : "/temp_run.txt");
        if (!fileController->saveFile(fileName, editor->getText())) {
            qDebug() << "Не удалось сохранить временный файл для выполнения:" << fileName;
            return;
        }
    }

    fileController->runScript(fileName);

    if (fileName.endsWith("temp_run.txt") || fileName.endsWith("temp_run.com")) {
        QFile::remove(fileName);
    }
}

void MainWindow::showSettingsDialog() {
    SettingsDialog* dialog = new SettingsDialog(this);
    dialog->setSettings(settingsManager->loadSettings());
    connect(dialog, &SettingsDialog::saveSettingsRequested, settingsManager, &SettingsManager::saveSettings);
    connect(dialog, &SettingsDialog::saveSettingsRequested, this, &MainWindow::updateEditors);
    connect(dialog, &SettingsDialog::languageChanged, this, &MainWindow::onLanguageChanged);
    connect(dialog, &SettingsDialog::saveSettingsRequested, this, [this](const QMap<QString, QVariant>& settings) {
        bool autoSave = settings["autoSave"].toBool();
        for (int i = 0; i < tabWidget->count(); ++i) {
            CodeEditor* editor = qobject_cast<CodeEditor*>(tabWidget->widget(i));
            if (editor) {
                disconnect(editor, &QPlainTextEdit::textChanged, this, &MainWindow::handleTextChanged);
                if (autoSave) {
                    connect(editor, &QPlainTextEdit::textChanged, this, &MainWindow::handleTextChanged);
                }
            }
        }
    });
    dialog->exec();
    delete dialog;
}

void MainWindow::updateEditors(const QMap<QString, QVariant>& settings) {
    for (int i = 0; i < tabWidget->count(); ++i) {
        CodeEditor* editor = qobject_cast<CodeEditor*>(tabWidget->widget(i));
        if (editor) {
            editor->setTheme(settings["theme"].toString(),
                             settings["backgroundColor"].value<QColor>(),
                             settings["textColor"].value<QColor>(),
                             settings["highlightColor"].value<QColor>());
            editor->setFont(settings["font"].value<QFont>());
            editor->setStandardLineNumbering(settings["standardLineNumbering"].toBool());
            editor->setAddressLineNumbering(settings["addressLineNumbering"].toBool());
            editor->setLineWrap(settings["lineWrap"].toBool());
            editor->setSyntaxHighlighting(settings["syntaxHighlighting"].toBool());
            editor->setShowMemoryDump(settings["showMemoryDump"].toBool(),
                                      settings["memoryDumpSegment"].toString(),
                                      settings["memoryDumpOffset"].toString(),
                                      settings["memoryDumpLineCount"].toInt());
        }
    }
}

void MainWindow::onLanguageChanged(const QString& language) {
    bool hasUnsavedChanges = false;
    for (int i = 0; i < tabWidget->count(); ++i) {
        CodeEditor* editor = qobject_cast<CodeEditor*>(tabWidget->widget(i));
        if (editor && editor->document()->isModified()) {
            hasUnsavedChanges = true;
            break;
        }
    }

    if (hasUnsavedChanges) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            tr("Unsaved Changes"),
            tr("You have unsaved changes. Would you like to save them before restarting?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
            );

        if (reply == QMessageBox::Save) {
            for (int i = 0; i < tabWidget->count(); ++i) {
                CodeEditor* editor = qobject_cast<CodeEditor*>(tabWidget->widget(i));
                if (editor && editor->document()->isModified()) {
                    tabWidget->setCurrentIndex(i);
                    saveFile();
                }
            }
        } else if (reply == QMessageBox::Cancel) {
            return;
        }
    }

    QMap<QString, QVariant> settings = settingsManager->loadSettings();
    settings["language"] = language;
    settingsManager->saveSettings(settings);
    QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
    qApp->quit();
}

void MainWindow::handleTextChanged() {
    QMap<QString, QVariant> settings = settingsManager->loadSettings();
    if (settings["autoSave"].toBool()) {
        saveFile();
    }
}
