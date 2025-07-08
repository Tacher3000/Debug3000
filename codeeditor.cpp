#include "codeeditor.h"

CodeEditor::SyntaxHighlighter::SyntaxHighlighter(QTextDocument* parent) : QSyntaxHighlighter(parent) {}

void CodeEditor::SyntaxHighlighter::highlightBlock(const QString& text) {
    QTextCharFormat instructionFormat;
    instructionFormat.setForeground(Qt::blue);
    QRegularExpression instructionRegex("\\b(mov|int|add|sub|jmp|call|ret|push|pop)\\b", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatchIterator it = instructionRegex.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        setFormat(match.capturedStart(), match.capturedLength(), instructionFormat);
    }

    QTextCharFormat commandFormat;
    commandFormat.setForeground(Qt::darkGreen);
    QRegularExpression commandRegex("-\\w+");
    it = commandRegex.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        setFormat(match.capturedStart(), match.capturedLength(), commandFormat);
    }
}

CodeEditor::CodeEditor(QWidget* parent) : QPlainTextEdit(parent), standardLineNumbering(true), addressLineNumbering(true), currentLineHighlight(true), lineWrap(false), tabSize(4) {
    lineNumberArea = new LineNumberArea(this);
    highlighter = new SyntaxHighlighter(document());
    setFont(QFont("Courier New", 10));
    setTabStopDistance(4 * fontMetrics().horizontalAdvance(' '));
    connect(this, &QPlainTextEdit::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest, this, &CodeEditor::updateLineNumberArea);
    connect(this, &QPlainTextEdit::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);
    updateLineNumberAreaWidth();
    highlightCurrentLine();
    lineNumberArea->setVisible(true);
}

CodeEditor::~CodeEditor() {
    delete lineNumberArea;
    delete highlighter;
}

void CodeEditor::setSyntaxHighlighting(const QString& theme) {}
void CodeEditor::setStandardLineNumbering(bool enabled) {
    standardLineNumbering = enabled;
    updateLineNumberAreaWidth();
    update();
}
void CodeEditor::setAddressLineNumbering(bool enabled) {
    addressLineNumbering = enabled;
    updateLineNumberAreaWidth();
    update();
}
void CodeEditor::setCurrentLineHighlight(bool enabled) {
    currentLineHighlight = enabled;
    highlightCurrentLine();
}
void CodeEditor::setTheme(const QString& theme) {
    this->theme = theme;
    // TODO: Реализовать изменение темы
}
void CodeEditor::setFont(const QFont& font) {
    QPlainTextEdit::setFont(font);
    updateLineNumberAreaWidth();
    update();
}
void CodeEditor::setLineWrap(bool enabled) {
    setLineWrapMode(enabled ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
    update();
}
void CodeEditor::setEncoding(const QString& encoding) {
    this->encoding = encoding;
    // TODO: Реализовать применение кодировки
}
void CodeEditor::setTabKey(int size) {
    tabSize = size;
    setTabStopDistance(size * fontMetrics().horizontalAdvance(' '));
    update();
}
QString CodeEditor::getText() const { return toPlainText(); }
void CodeEditor::setText(const QString& text) { setPlainText(text); }
void CodeEditor::calculateAddresses() {}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent* event) {
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString standardNumber = standardLineNumbering ? QString::number(blockNumber + 1) : "";
            QString addressNumber;
            if (addressLineNumbering && (block.text().trimmed().startsWith("-a") || block.text().trimmed().split(" ").first().toLower() == "mov")) {
                addressNumber = QString(":%1").arg(blockNumber * 2 + 0x100, 4, 16, QChar('0')).toUpper();
            }
            painter.setPen(Qt::black);
            painter.drawText(0, top, lineNumberArea->width() / 2 - 5, fontMetrics().height(), Qt::AlignRight, standardNumber);
            painter.drawText(lineNumberArea->width() / 2, top, lineNumberArea->width() / 2 - 5, fontMetrics().height(), Qt::AlignRight, addressNumber);
        }
        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

void CodeEditor::resizeEvent(QResizeEvent* event) {
    QPlainTextEdit::resizeEvent(event);
    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::updateLineNumberAreaWidth() {
    int width = lineNumberAreaWidth();
    setViewportMargins(width, 0, 0, 0);
    lineNumberArea->update();
}

int CodeEditor::lineNumberAreaWidth() {
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) { max /= 10; ++digits; }
    int standardWidth = standardLineNumbering ? fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits : 0;
    int addressWidth = addressLineNumbering ? fontMetrics().horizontalAdvance(":0000") : 0;
    int space = qMax(60, 10 + standardWidth + addressWidth + (standardLineNumbering && addressLineNumbering ? 20 : 0));
    return space;
}

void CodeEditor::updateLineNumberArea(const QRect& rect, int dy) {
    if (dy) {
        lineNumberArea->scroll(0, dy);
    } else {
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
    }
    if (rect.contains(viewport()->rect())) {
        updateLineNumberAreaWidth();
    }
}

void CodeEditor::highlightCurrentLine() {
    QList<QTextEdit::ExtraSelection> extraSelections;
    if (currentLineHighlight) {
        QTextEdit::ExtraSelection selection;
        selection.format.setBackground(QColor(Qt::darkGray).lighter(160));
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }
    setExtraSelections(extraSelections);
}
