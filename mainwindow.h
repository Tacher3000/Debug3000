#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QMap>
#include <QVariant>
#include <QTranslator>
#include <QWebEngineView>
#include "codeeditor.h"
#include "settingsmanager.h"
#include "settingsdialog.h"
#include "filecontroller.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
protected:
    void closeEvent(QCloseEvent* event) override;
private slots:
    void newFile();
    void openFile();
    void saveFile();
    void saveFileAs();
    void runCode();
    void showSettingsDialog();
    void showHelp();
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
    QAction* helpAction;
    QMenu* fileMenu;
    QMenu* settingsMenu;
    QMenu* helpMenu;
    QToolBar* toolBar;
    QWebEngineView* helpView;
    QMainWindow* helpWindow;
    void createMenus();
    void createToolBar();
    void updateInterfaceTranslations();
    CodeEditor* getCurrentEditor() const;
};

#endif // MAINWINDOW_H
