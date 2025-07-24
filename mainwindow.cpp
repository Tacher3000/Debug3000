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
#include <QDesktopServices>
#include <QUrl>
#include <QWebEngineView>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), isLanguageChangeClosing(false) {
    setWindowTitle(tr("Debug3000"));
    tabWidget = new QTabWidget(this);
    tabWidget->setTabsClosable(true);
    setCentralWidget(tabWidget);
    settingsManager = new SettingsManager(this);
    fileController = new FileController(this);
    helpView = nullptr;
    helpWindow = nullptr;
    createMenus();
    createToolBar();
    connect(settingsManager, &SettingsManager::settingsChanged, this, &MainWindow::updateEditors);
    connect(fileController, &FileController::compileAndRunFinished, this, &MainWindow::onCompileAndRunFinished);
    connect(tabWidget, &QTabWidget::tabCloseRequested, this, [this](int index) {
        if (promptSaveChanges(index)) {
            editorTabs.remove(index);
            tabWidget->removeTab(index);
            QMap<int, EditorTab> updatedTabs;
            for (int i = 0, newIndex = 0; i < tabWidget->count(); ++i, ++newIndex) {
                updatedTabs[newIndex] = editorTabs[i];
            }
            editorTabs = updatedTabs;
        }
    });
    settingsManager->applySettings();
}

MainWindow::~MainWindow() {
    delete helpWindow;
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (isLanguageChangeClosing) {
        event->accept();
        return;
    }

    for (int i = 0; i < tabWidget->count(); ++i) {
        if (!promptSaveChanges(i)) {
            event->ignore();
            return;
        }
    }
    event->accept();
}

void MainWindow::createMenus() {
    fileMenu = menuBar()->addMenu(tr("File"));
    newAction = fileMenu->addAction(tr("New"));
    newAction->setShortcut(QKeySequence::New);
    openAction = fileMenu->addAction(tr("Open"));
    openAction->setShortcut(QKeySequence::Open);
    saveAction = fileMenu->addAction(tr("Save"));
    saveAction->setShortcut(QKeySequence::Save);
    saveAsAction = fileMenu->addAction(tr("Save As"));
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    pasteCodeAction = fileMenu->addAction(tr("Paste Code"));
    pasteCodeAction->setShortcut(Qt::Key_F6);
    runAction = fileMenu->addAction(tr("Run"));
    runAction->setShortcut(Qt::Key_F5);
    settingsMenu = menuBar()->addMenu(tr("Settings"));
    settingsAction = settingsMenu->addAction(tr("Preferences"));
    helpMenu = menuBar()->addMenu(tr("Help"));
    helpAction = helpMenu->addAction(tr("About Debug3000"));
    connect(newAction, &QAction::triggered, this, &MainWindow::newFile);
    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveFile);
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveFileAs);
    connect(pasteCodeAction, &QAction::triggered, this, &MainWindow::pasteCode);
    connect(runAction, &QAction::triggered, this, &MainWindow::run);
    connect(settingsAction, &QAction::triggered, this, &MainWindow::showSettingsDialog);
    connect(helpAction, &QAction::triggered, this, &MainWindow::showHelp);
}

void MainWindow::createToolBar() {
    toolBar = addToolBar(tr("Tools"));
    toolBar->addAction(newAction);
    toolBar->addAction(openAction);
    toolBar->addAction(saveAction);
    toolBar->addAction(saveAsAction);
    toolBar->addAction(pasteCodeAction);
    toolBar->addAction(runAction);
    toolBar->addAction(settingsAction);
    toolBar->addAction(helpAction);
}

void MainWindow::updateInterfaceTranslations() {
    setWindowTitle(tr("Debug3000"));
    fileMenu->setTitle(tr("File"));
    newAction->setText(tr("New"));
    openAction->setText(tr("Open"));
    saveAction->setText(tr("Save"));
    saveAsAction->setText(tr("Save As"));
    pasteCodeAction->setText(tr("Paste Code"));
    runAction->setText(tr("Run"));
    settingsMenu->setTitle(tr("Settings"));
    settingsAction->setText(tr("Preferences"));
    helpMenu->setTitle(tr("Help"));
    helpAction->setText(tr("About Debug3000"));
    toolBar->setWindowTitle(tr("Tools"));
    for (int i = 0; i < tabWidget->count(); ++i) {
        if (tabWidget->tabText(i) == "New File" || tabWidget->tabText(i) == tr("New File")) {
            tabWidget->setTabText(i, tr("New File"));
        }
    }
}

CodeEditor* MainWindow::getCurrentEditor() const {
    int index = tabWidget->currentIndex();
    if (editorTabs.contains(index)) {
        return editorTabs[index].editor;
    }
    return nullptr;
}

void MainWindow::newFile() {
    QMap<QString, QVariant> settings = settingsManager->loadSettings();
    CodeEditor* currentEditor = getCurrentEditor();
    if (currentEditor && currentEditor->document()->isModified()) {
        if (!promptSaveChanges(tabWidget->currentIndex())) {
            return;
        }
    }

    CodeEditor* editor = new CodeEditor();
    QSplitter* splitter = new QSplitter(Qt::Vertical);
    QTextEdit* outputConsole = new QTextEdit();
    outputConsole->setReadOnly(true);
    outputConsole->setMinimumHeight(100);
    splitter->addWidget(editor);
    splitter->addWidget(outputConsole);
    splitter->setSizes({400, 100});
    outputConsole->setVisible(settings["showOutputConsole"].toBool());
    tabWidget->addTab(splitter, tr("New File"));
    tabWidget->setCurrentWidget(splitter);
    EditorTab tab = {editor, splitter, outputConsole, "", false};
    editorTabs[tabWidget->currentIndex()] = tab;
    updateTab(tabWidget->currentIndex(), settings);
    if (settings["autoSave"].toBool()) {
        connect(editor, &QPlainTextEdit::textChanged, this, &MainWindow::handleTextChanged);
    }
}

void MainWindow::openFile() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("Text and COM Files (*.txt *.com *.COM);;All Files (*)"));
    if (fileName.isEmpty()) return;

    for (int i = 0; i < tabWidget->count(); ++i) {
        if (editorTabs[i].filePath == fileName) {
            tabWidget->setCurrentIndex(i);
            return;
        }
    }

    CodeEditor* currentEditor = getCurrentEditor();
    if (currentEditor && currentEditor->document()->isModified()) {
        if (!promptSaveChanges(tabWidget->currentIndex())) {
            return;
        }
    }

    QString content = fileController->openFile(fileName);
    CodeEditor* editor = new CodeEditor();
    editor->setText(content);
    bool isComFile = fileName.endsWith(".com", Qt::CaseInsensitive);
    if (isComFile) {
        editor->setReadOnly(true);
    }
    QSplitter* splitter = new QSplitter(Qt::Vertical);
    QTextEdit* outputConsole = new QTextEdit();
    outputConsole->setReadOnly(true);
    outputConsole->setMinimumHeight(100);
    splitter->addWidget(editor);
    splitter->addWidget(outputConsole);
    splitter->setSizes({400, 100});
    QMap<QString, QVariant> settings = settingsManager->loadSettings();
    outputConsole->setVisible(settings["showOutputConsole"].toBool());
    tabWidget->addTab(splitter, QFileInfo(fileName).fileName());
    tabWidget->setCurrentWidget(splitter);
    EditorTab tab = {editor, splitter, outputConsole, fileName, isComFile};
    editorTabs[tabWidget->currentIndex()] = tab;
    updateTab(tabWidget->currentIndex(), settings);
    if (settings["autoSave"].toBool() && !isComFile) {
        connect(editor, &QPlainTextEdit::textChanged, this, &MainWindow::handleTextChanged);
    }
}

void MainWindow::saveFile() {
    CodeEditor* editor = getCurrentEditor();
    if (!editor) return;

    int index = tabWidget->currentIndex();
    if (editorTabs[index].isReadOnly) {
        QMessageBox::warning(this, tr("Read-Only File"), tr("Cannot save a read-only COM file."));
        return;
    }

    QString fileName = editorTabs[index].filePath;
    if (fileName.isEmpty() || tabWidget->tabText(index) == tr("New File")) {
        saveFileAs();
        return;
    }

    if (fileController->saveFile(fileName, editor->getText())) {
        tabWidget->setTabText(index, QFileInfo(fileName).fileName());
        editor->document()->setModified(false);
        editorTabs[index].filePath = fileName;
    }
}

void MainWindow::saveFileAs() {
    CodeEditor* editor = getCurrentEditor();
    if (!editor) return;

    int index = tabWidget->currentIndex();
    if (editorTabs[index].isReadOnly) {
        QMessageBox::warning(this, tr("Read-Only File"), tr("Cannot save a read-only COM file."));
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File As"), "", tr("Text Files (*.txt);;All Files (*)"));
    if (!fileName.isEmpty()) {
        if (fileController->saveAsFile(fileName, editor->getText())) {
            tabWidget->setTabText(index, QFileInfo(fileName).fileName());
            editorTabs[index].filePath = fileName;
            editorTabs[index].isReadOnly = false;
            editor->setReadOnly(false);
            editor->document()->setModified(false);
        }
    }
}

void MainWindow::pasteCode() {
    CodeEditor* editor = getCurrentEditor();
    if (!editor) return;

    int index = tabWidget->currentIndex();
    if (editorTabs[index].isReadOnly) {
        QMessageBox::warning(this, tr("Read-Only File"), tr("Cannot paste code into a read-only COM file."));
        return;
    }

    QString fileName = editorTabs[index].filePath;
    if (fileName.isEmpty() || tabWidget->tabText(index) == tr("New File")) {
        fileName = QCoreApplication::applicationDirPath() + "/temp_paste.txt";
        if (!fileController->saveFile(fileName, editor->getText())) {
            qDebug() << "Failed to save temporary file for pasting:" << fileName;
            return;
        }
        editorTabs[index].filePath = fileName;
    } else {
        if (!fileController->saveFile(fileName, editor->getText())) {
            qDebug() << "Failed to save file for pasting:" << fileName;
            return;
        }
    }

    fileController->pasteCodeToDebug(fileName);

    if (fileName.endsWith("temp_paste.txt")) {
        QFile::remove(fileName);
        editorTabs[index].filePath = "";
    }
}

void MainWindow::run() {
    CodeEditor* editor = getCurrentEditor();
    if (!editor) return;

    int index = tabWidget->currentIndex();
    QString fileName = editorTabs[index].filePath;

    if (editorTabs[index].isReadOnly && !fileName.endsWith(".com", Qt::CaseInsensitive)) {
        QMessageBox::warning(this, tr("Invalid File"), tr("Read-only files must be COM files to run."));
        return;
    }

    QMap<QString, QVariant> settings = settingsManager->loadSettings();
    bool autoSave = settings["autoSave"].toBool();

    if (fileName.isEmpty() || tabWidget->tabText(index) == tr("New File")) {
        fileName = QCoreApplication::applicationDirPath() + "/temp_run.txt";
        editorTabs[index].filePath = fileName;
    } else if (editor->document()->isModified() && !autoSave) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            tr("Unsaved Changes"),
            tr("The file '%1' has unsaved changes. Would you like to save them before running?").arg(tabWidget->tabText(index)),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
            );

        if (reply == QMessageBox::Save) {
            saveFile();
            if (editor->document()->isModified()) {
                return;
            }
        } else if (reply == QMessageBox::Cancel) {
            return;
        }
        QString tempFileName = QCoreApplication::applicationDirPath() + "/temp_run.txt";
        if (!fileController->saveFile(tempFileName, editor->getText())) {
            qDebug() << "Failed to save temporary file for execution:" << tempFileName;
            QMessageBox::warning(this, tr("Save Error"), tr("Failed to save the temporary file for execution."));
            return;
        }
        fileName = tempFileName;
        editorTabs[index].filePath = tempFileName;
    }

    if (!fileController->saveFile(fileName, editor->getText())) {
        qDebug() << "Failed to save file for execution:" << fileName;
        QMessageBox::warning(this, tr("Save Error"), tr("Failed to save the file for execution."));
        return;
    }

    fileController->compileAndRunCom(fileName);
}

void MainWindow::onCompileAndRunFinished(const QString& output) {
    int index = tabWidget->currentIndex();
    if (editorTabs.contains(index)) {
        QString fileName = editorTabs[index].filePath;
        updateOutputConsole(index, output);
        if (fileName.endsWith("temp_run.txt")) {
            QFile::remove(fileName);
            editorTabs[index].filePath = "";
        }
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
            CodeEditor* editor = editorTabs[i].editor;
            if (editor && !editorTabs[i].isReadOnly) {
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

void MainWindow::showHelp() {
    if (!helpWindow) {
        helpWindow = new QMainWindow(this);
        helpWindow->setWindowTitle(tr("Debug3000 Help"));
        helpWindow->setMinimumSize(800, 600);

        helpView = new QWebEngineView(helpWindow);
        QVBoxLayout* layout = new QVBoxLayout;
        layout->addWidget(helpView);
        QWidget* centralWidget = new QWidget;
        centralWidget->setLayout(layout);
        helpWindow->setCentralWidget(centralWidget);

        helpView->load(QUrl("qrc:/help/help.html"));
        connect(helpView, &QWebEngineView::loadFinished, this, [this](bool ok) {
            if (!ok) {
                QMessageBox::warning(this, tr("Error"), tr("Failed to load help file."));
            }
        });
    }
    helpWindow->show();
    helpWindow->raise();
    helpWindow->activateWindow();
}

void MainWindow::updateEditors(const QMap<QString, QVariant>& settings) {
    for (int i = 0; i < tabWidget->count(); ++i) {
        updateTab(i, settings);
    }
}

void MainWindow::onLanguageChanged(const QString& language) {
    for (int i = 0; i < tabWidget->count(); ++i) {
        if (!promptSaveChanges(i)) {
            return;
        }
    }

    QMap<QString, QVariant> settings = settingsManager->loadSettings();
    settings["language"] = language;
    settingsManager->saveSettings(settings);
    updateInterfaceTranslations();
    isLanguageChangeClosing = true;
    QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
    close();
}

void MainWindow::handleTextChanged() {
    QMap<QString, QVariant> settings = settingsManager->loadSettings();
    if (settings["autoSave"].toBool()) {
        saveFile();
    }
}

void MainWindow::updateTab(int index, const QMap<QString, QVariant>& settings) {
    EditorTab& tab = editorTabs[index];
    if (tab.editor) {
        tab.editor->setTheme(settings["theme"].toString(),
                             settings["backgroundColor"].value<QColor>(),
                             settings["textColor"].value<QColor>(),
                             settings["highlightColor"].value<QColor>());
        tab.editor->setFont(settings["font"].value<QFont>());
        tab.editor->setStandardLineNumbering(settings["standardLineNumbering"].toBool());
        tab.editor->setAddressLineNumbering(settings["addressLineNumbering"].toBool());
        tab.editor->setLineWrap(settings["lineWrap"].toBool());
        tab.editor->setSyntaxHighlighting(settings["syntaxHighlighting"].toBool());
        tab.editor->setShowMemoryDump(settings["showMemoryDump"].toBool(),
                                      settings["memoryDumpSegment"].toString(),
                                      settings["memoryDumpOffset"].toString(),
                                      settings["memoryDumpLineCount"].toInt());
        tab.outputConsole->setVisible(settings["showOutputConsole"].toBool());
    }
}

void MainWindow::updateOutputConsole(int index, const QString& output) {
    if (editorTabs.contains(index)) {
        editorTabs[index].outputConsole->setPlainText(output);
    }
}

bool MainWindow::promptSaveChanges(int index) {
    if (!editorTabs.contains(index) || !editorTabs[index].editor->document()->isModified() || editorTabs[index].isReadOnly) {
        return true;
    }

    QString tabName = tabWidget->tabText(index);
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        tr("Unsaved Changes"),
        tr("The file '%1' has unsaved changes. Would you like to save them?").arg(tabName),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
        );

    if (reply == QMessageBox::Save) {
        tabWidget->setCurrentIndex(index);
        saveFile();
        return !editorTabs[index].editor->document()->isModified();
    } else if (reply == QMessageBox::Discard) {
        return true;
    } else {
        return false;
    }
}
