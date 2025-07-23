#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QMap>
#include <QVariant>
#include <QTranslator>
#include <QWebEngineView>
#include <QSplitter>
#include <QTextEdit>
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
    void pasteCode();
    void run();
    void onCompileAndRunFinished(const QString& output);
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
    QAction* pasteCodeAction;
    QAction* runAction;
    QAction* settingsAction;
    QAction* helpAction;
    QMenu* fileMenu;
    QMenu* settingsMenu;
    QMenu* helpMenu;
    QToolBar* toolBar;
    QWebEngineView* helpView;
    QMainWindow* helpWindow;
    bool isLanguageChangeClosing;
    bool promptSaveChanges(int index);
    void createMenus();
    void createToolBar();
    void updateInterfaceTranslations();
    CodeEditor* getCurrentEditor() const;

    struct EditorTab {
        CodeEditor* editor;
        QSplitter* splitter;
        QTextEdit* outputConsole;
        QString filePath;
        bool isReadOnly;
    };
    QMap<int, EditorTab> editorTabs;
    void updateOutputConsole(int index, const QString& output);
    void updateTab(int index, const QMap<QString, QVariant>& settings);
};

#endif // MAINWINDOW_H
