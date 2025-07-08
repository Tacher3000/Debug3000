#include "mainwindow.h"
#include <QMenuBar>
#include <QToolBar>
#include <QAction>
#include <QFileDialog>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("DebugCrafter");
    tabWidget = new QTabWidget(this);
    tabWidget->setTabsClosable(true);
    setCentralWidget(tabWidget);
    settingsManager = new SettingsManager();
    fileController = new FileController();
    createMenus();
    createToolBar();
    connect(settingsManager, &SettingsManager::settingsChanged, this, &MainWindow::updateEditors);
    connect(tabWidget, &QTabWidget::tabCloseRequested, this, [this](int index) { tabWidget->removeTab(index); });
    settingsManager->applySettings();
}

MainWindow::~MainWindow() {
    delete settingsManager;
    delete fileController;
}

void MainWindow::createMenus() {
    QMenu* fileMenu = menuBar()->addMenu("Файл");
    QAction* newAction = fileMenu->addAction("Новый");
    QAction* openAction = fileMenu->addAction("Открыть");
    QAction* saveAction = fileMenu->addAction("Сохранить");
    QAction* runAction = fileMenu->addAction("Запустить");
    QMenu* settingsMenu = menuBar()->addMenu("Настройки");
    QAction* preferencesAction = settingsMenu->addAction("Параметры");
    connect(newAction, &QAction::triggered, this, &MainWindow::newFile);
    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveFile);
    connect(runAction, &QAction::triggered, this, &MainWindow::runCode);
    connect(preferencesAction, &QAction::triggered, this, &MainWindow::showSettingsDialog);
}

void MainWindow::createToolBar() {
    QToolBar* toolBar = addToolBar("Tools");
    QAction* newAction = toolBar->addAction(QIcon(":/icons/new.png"), "Новый");
    QAction* openAction = toolBar->addAction(QIcon(":/icons/open.png"), "Открыть");
    QAction* saveAction = toolBar->addAction(QIcon(":/icons/save.png"), "Сохранить");
    QAction* runAction = toolBar->addAction(QIcon(":/icons/run.png"), "Запустить");
    QAction* settingsAction = toolBar->addAction(QIcon(":/icons/settings.png"), "Настройки");
    connect(newAction, &QAction::triggered, this, &MainWindow::newFile);
    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveFile);
    connect(runAction, &QAction::triggered, this, &MainWindow::runCode);
    connect(settingsAction, &QAction::triggered, this, &MainWindow::showSettingsDialog);
}

CodeEditor* MainWindow::getCurrentEditor() const {
    return qobject_cast<CodeEditor*>(tabWidget->currentWidget());
}

void MainWindow::newFile() {
    CodeEditor* editor = new CodeEditor();
    tabWidget->addTab(editor, "Новый файл");
    tabWidget->setCurrentWidget(editor);
    updateEditors(settingsManager->loadSettings());
}

void MainWindow::openFile() {
    QString fileName = QFileDialog::getOpenFileName(this, "Открыть файл", "", "Text Files (*.txt);;All Files (*)");
    if (!fileName.isEmpty()) {
        QString content = fileController->openFile(fileName);
        CodeEditor* editor = new CodeEditor();
        editor->setText(content);
        tabWidget->addTab(editor, QFileInfo(fileName).fileName());
        tabWidget->setCurrentWidget(editor);
        updateEditors(settingsManager->loadSettings());
    }
}

void MainWindow::saveFile() {
}

void MainWindow::runCode() {
}

void MainWindow::showSettingsDialog() {
    SettingsDialog* dialog = new SettingsDialog(this);
    dialog->setSettings(settingsManager->loadSettings());
    connect(dialog, &SettingsDialog::saveSettingsRequested, settingsManager, &SettingsManager::saveSettings);
    dialog->exec();
    delete dialog;
}

void MainWindow::updateEditors(const QMap<QString, QVariant>& settings) {
    for (int i = 0; i < tabWidget->count(); ++i) {
        CodeEditor* editor = qobject_cast<CodeEditor*>(tabWidget->widget(i));
        if (editor) {
            editor->setTheme(settings["theme"].toString());
            editor->setFont(settings["font"].value<QFont>());
            editor->setStandardLineNumbering(settings["standardLineNumbering"].toBool());
            editor->setAddressLineNumbering(settings["addressLineNumbering"].toBool());
            editor->setLineWrap(settings["lineWrap"].toBool());
            editor->setEncoding(settings["encoding"].toString());
            editor->setTabKey(settings["tabSize"].toInt());
        }
    }
}
