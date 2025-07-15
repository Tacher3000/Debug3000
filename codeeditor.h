#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QFont>
#include <QString>
#include <QSyntaxHighlighter>
#include <QPainter>
#include <QTextBlock>
#include <QRegularExpression>
#include <QColor>
#include <QCompleter>

class LineNumberArea;

class CodeEditor : public QPlainTextEdit {
    Q_OBJECT
protected:
    void resizeEvent(QResizeEvent* event) override;
private slots:
    void updateLineNumberArea(const QRect& rect, int dy);
    void highlightCurrentLine();
public:
    CodeEditor(QWidget* parent = nullptr);
    ~CodeEditor();
    void setStandardLineNumbering(bool enabled);
    void setAddressLineNumbering(bool enabled);
    void setCurrentLineHighlight(bool enabled);
    void setTheme(const QString& theme, const QColor& backgroundColor, const QColor& textColor, const QColor& highlightColor);
    void setFont(const QFont& font);
    void setLineWrap(bool enabled);
    void setAutoComplete(bool enabled);
    void setSyntaxHighlighting(bool enabled);
    QString getText() const;
    void setText(const QString& text);
    void lineNumberAreaPaintEvent(QPaintEvent* event);
    int lineNumberAreaWidth();
private:
    static const int MARGIN_LEFT = 5;
    static const int MARGIN_RIGHT = 10;
    static const int ADDRESS_EXTRA_WIDTH = 15;
    static const QString ADDRESS_FORMAT;

    QString theme;
    QFont font;
    bool standardLineNumbering;
    bool addressLineNumbering;
    bool currentLineHighlight;
    bool lineWrap;
    bool autoComplete;
    bool syntaxHighlighting;
    LineNumberArea* lineNumberArea;
    QSyntaxHighlighter* highlighter;
    QCompleter* completer;
    QColor backgroundColor;
    QColor textColor;
    QColor highlightColor;

    class SyntaxHighlighter : public QSyntaxHighlighter {
    public:
        SyntaxHighlighter(QTextDocument* parent);
        void setEnabled(bool enabled);
    protected:
        void highlightBlock(const QString& text) override;
    private:
        bool enabled;
    };

    void updateLineNumberAreaWidth();
    int calculateStandardWidth() const;
    int calculateAddressWidth() const;
    int calculateInstructionLength(const QString &text);
    void setupAutoComplete();
};

class LineNumberArea : public QWidget {
public:
    LineNumberArea(CodeEditor* editor) : QWidget(editor), codeEditor(editor) {}
    QSize sizeHint() const override {
        return QSize(codeEditor->lineNumberAreaWidth(), 0);
    }
protected:
    void paintEvent(QPaintEvent* event) override {
        codeEditor->lineNumberAreaPaintEvent(event);
    }
private:
    CodeEditor* codeEditor;
};

#endif // CODEEDITOR_H
