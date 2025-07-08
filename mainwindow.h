#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QMap>
#include <QVariant>
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
    void runCode();
    void showSettingsDialog();
    void updateEditors(const QMap<QString, QVariant>& settings);
private:
    QTabWidget* tabWidget;
    SettingsManager* settingsManager;
    FileController* fileController;
    void createMenus();
    void createToolBar();
    CodeEditor* getCurrentEditor() const;
};

#endif // MAINWINDOW_H
