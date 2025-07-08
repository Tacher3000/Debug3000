#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QFont>
#include <QString>
#include <QSyntaxHighlighter>
#include <QPainter>
#include <QTextBlock>
#include <QRegularExpression>
#include <QTextCharFormat>
#include <QColor>

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
    void setSyntaxHighlighting(const QString& theme);
    void setStandardLineNumbering(bool enabled);
    void setAddressLineNumbering(bool enabled);
    void setCurrentLineHighlight(bool enabled);
    void setTheme(const QString& theme);
    void setFont(const QFont& font);
    void setLineWrap(bool enabled);
    void setEncoding(const QString& encoding);
    void setTabKey(int size);
    QString getText() const;
    void setText(const QString& text);
    void calculateAddresses();
    void lineNumberAreaPaintEvent(QPaintEvent* event);
    int lineNumberAreaWidth();
private:
    QString text;
    QString theme;
    QFont font;
    bool standardLineNumbering;
    bool addressLineNumbering;
    bool currentLineHighlight;
    bool lineWrap;
    QString encoding;
    int tabSize;
    LineNumberArea* lineNumberArea;
    QSyntaxHighlighter* highlighter;

    class SyntaxHighlighter : public QSyntaxHighlighter {
    public:
        SyntaxHighlighter(QTextDocument* parent);
    protected:
        void highlightBlock(const QString& text) override;
    };

    void updateLineNumberAreaWidth();
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
