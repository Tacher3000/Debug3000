#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QMap>
#include <QVariant>
#include <QTranslator>
#include "codeeditor.h"
#include "settingsmanager.h"
#include "settingsdialog.h"
#include "filecontroller.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
private slots:
    void newFile();
    void openFile();
    void saveFile();
    void saveFileAs();
    void runCode();
    void showSettingsDialog();
    void updateEditors(const QMap<QString, QVariant>& settings);
    void onLanguageChanged(const QString& language);
    void handleTextChanged();
private:
    QTabWidget* tabWidget;
    SettingsManager* settingsManager;
    FileController* fileController;
    QTranslator translator;
    QAction* newAction;
    QAction* openAction;
    QAction* saveAction;
    QAction* saveAsAction;
    QAction* runAction;
    QAction* settingsAction;
    QMenu* fileMenu;
    QMenu* settingsMenu;
    QToolBar* toolBar;
    void createMenus();
    void createToolBar();
    void updateInterfaceTranslations();
    CodeEditor* getCurrentEditor() const;
};

#endif // MAINWINDOW_H
