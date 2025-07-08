#include "codeeditor.h"
#include <QRegularExpression>
#include <QTextBlock>
#include <QPainter>
#include <QFontMetrics>

CodeEditor::SyntaxHighlighter::SyntaxHighlighter(QTextDocument* parent) : QSyntaxHighlighter(parent) {}

void CodeEditor::SyntaxHighlighter::highlightBlock(const QString& text) {
    QTextCharFormat instructionFormat;
    instructionFormat.setForeground(Qt::blue);
    QRegularExpression instructionRegex("\\b(mov|int|add|sub|cmp|jmp|je|jne|jz|jnz|jg|jge|jl|jle|mul|div|imul|idiv|inc|dec|push|pop|call|ret)\\b", QRegularExpression::CaseInsensitiveOption);
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

    QTextCharFormat addressFormat;
    addressFormat.setForeground(Qt::red);
    QRegularExpression addressRegex("\\ba\\s+[0-9A-Fa-f]+\\b");
    it = addressRegex.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        setFormat(match.capturedStart(), match.capturedLength(), addressFormat);
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
    lineNumberArea->update();
}
void CodeEditor::setAddressLineNumbering(bool enabled) {
    addressLineNumbering = enabled;
    updateLineNumberAreaWidth();
    lineNumberArea->update();
}
void CodeEditor::setCurrentLineHighlight(bool enabled) {
    currentLineHighlight = enabled;
    highlightCurrentLine();
}
void CodeEditor::setTheme(const QString& theme) {
    this->theme = theme;
}
void CodeEditor::setFont(const QFont& font) {
    QPlainTextEdit::setFont(font);
    updateLineNumberAreaWidth();
    lineNumberArea->update();
}
void CodeEditor::setLineWrap(bool enabled) {
    setLineWrapMode(enabled ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
    lineNumberArea->update();
}
void CodeEditor::setEncoding(const QString& encoding) {
    this->encoding = encoding;
}
void CodeEditor::setTabKey(int size) {
    tabSize = size;
    setTabStopDistance(size * fontMetrics().horizontalAdvance(' '));
    lineNumberArea->update();
}
QString CodeEditor::getText() const { return toPlainText(); }
void CodeEditor::setText(const QString& text) { setPlainText(text); }
void CodeEditor::calculateAddresses() {}

int CodeEditor::calculateInstructionLength(const QString& text) {
    QString trimmed = text.trimmed().toUpper();
    if (trimmed.isEmpty() || trimmed.startsWith("A ")) return 0;

    QStringList parts = trimmed.split(QRegularExpression("\\s+|,"), Qt::SkipEmptyParts);
    if (parts.isEmpty()) return 0;

    QString instruction = parts[0].toUpper();
    QString arg1 = parts.size() > 1 ? parts[1].toUpper() : "";
    QString arg2 = parts.size() > 2 ? parts[2].toUpper() : "";

    auto isRegister = [](const QString& arg) {
        return QStringList{"AX", "BX", "CX", "DX", "AL", "AH", "BL", "BH", "CL", "CH", "DL", "DH", "SP", "BP", "SI", "DI"}.contains(arg);
    };
    auto isNumber = [](const QString& arg) {
        bool ok;
        arg.toInt(&ok, 16);
        return ok;
    };
    auto isMemory = [](const QString& arg) {
        return arg.startsWith("[") && arg.endsWith("]") &&
               QStringList{"BX", "SI", "DI"}.contains(arg.mid(1, arg.length() - 2).toUpper());
    };

    bool isReg1 = isRegister(arg1);
    bool isNum2 = isNumber(arg2);
    bool isReg2 = isRegister(arg2);
    bool isMem2 = isMemory(arg2);

    if (instruction == "MOV") {
        if (isReg1 && isNum2) {
            return (arg1 == "AL" || arg1 == "AH" || arg1 == "BL" || arg1 == "BH" ||
                    arg1 == "CL" || arg1 == "CH" || arg1 == "DL" || arg1 == "DH") ? 2 : 3;
        } else if (isReg1 && isReg2) {
            return 2;
        } else if (isReg1 && isMem2 || isMem2 && isReg1) {
            return 2;
        }
    } else if (instruction == "ADD" || instruction == "SUB" || instruction == "CMP") {
        if (isReg1 && isNum2) {
            return (arg2.toInt(nullptr, 16) <= 0xFF) ? 3 : 4;
        } else if (isReg1 && isReg2 || isReg1 && isMem2) {
            return 2;
        }
    } else if (instruction == "JMP" || instruction == "JE" || instruction == "JNE" ||
               instruction == "JZ" || instruction == "JNZ" || instruction == "JG" ||
               instruction == "JGE" || instruction == "JL" || instruction == "JLE") {
        if (isNumber(arg1)) {
            return  2;
        }
        return 2;
    } else if (instruction == "MUL" || instruction == "DIV" || instruction == "IMUL" || instruction == "IDIV") {
        return 2;
    } else if (instruction == "INC" || instruction == "DEC" || instruction == "PUSH" || instruction == "POP") {
        return 1;
    } else if (instruction == "INT") {
        return 2;
    }

    return 0;
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent* event) {
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    const int standardWidth = calculateStandardWidth();
    const int addressWidth = calculateAddressWidth();

    int currentAddress = -1;
    bool addressMode = false;
    int nextAddress = -1;
    bool showNextEmptyAddress = false;

    QTextBlock currentBlock = document()->begin();
    int currentBlockNumber = 0;

    while (currentBlock.isValid()) {
        QString trimmed = currentBlock.text().trimmed();

        if (addressLineNumbering) {
            if (trimmed.startsWith("a ", Qt::CaseInsensitive)) {
                QStringList parts = trimmed.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
                if (parts.size() >= 2) {
                    bool ok;
                    currentAddress = parts[1].toInt(&ok, 16);
                    if (ok) {
                        addressMode = true;
                        nextAddress = currentAddress;
                        showNextEmptyAddress = true;
                    }
                }
            }
            else if (trimmed.isEmpty() && addressMode && showNextEmptyAddress) {
                if (currentBlockNumber >= blockNumber && currentBlock.isVisible() && bottom >= event->rect().top()) {
                    QString addressText = QString(":%1").arg(nextAddress, 4, 16, QChar('0')).toUpper();
                    painter.setPen(Qt::black);
                    painter.drawText(standardWidth, top, addressWidth - 10, fontMetrics().height(),
                                     Qt::AlignRight | Qt::AlignVCenter, addressText);
                }
                addressMode = false;
                showNextEmptyAddress = false;
            }
            else if (addressMode && !trimmed.isEmpty()) {
                if (currentBlockNumber >= blockNumber && currentBlock.isVisible() && bottom >= event->rect().top()) {
                    QString addressText = QString(":%1").arg(nextAddress, 4, 16, QChar('0')).toUpper();
                    painter.setPen(Qt::black);
                    painter.drawText(standardWidth, top, addressWidth - 10, fontMetrics().height(),
                                     Qt::AlignRight | Qt::AlignVCenter, addressText);
                }
                nextAddress += calculateInstructionLength(trimmed);
            }
        }

        if (standardLineNumbering && currentBlockNumber >= blockNumber && currentBlock.isVisible() && bottom >= event->rect().top()) {
            painter.setPen(Qt::black);
            const QString number = QString::number(currentBlockNumber + 1);
            painter.drawText(5, top, standardWidth - 10, fontMetrics().height(),
                             Qt::AlignRight | Qt::AlignVCenter, number);
        }

        if (currentBlockNumber >= blockNumber) {
            top = bottom;
            bottom = top + qRound(blockBoundingRect(currentBlock).height());
        }
        currentBlock = currentBlock.next();
        ++currentBlockNumber;
    }
}

void CodeEditor::resizeEvent(QResizeEvent* event) {
    QPlainTextEdit::resizeEvent(event);
    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
    lineNumberArea->update();
}

void CodeEditor::updateLineNumberAreaWidth() {
    int width = lineNumberAreaWidth();
    setViewportMargins(width, 0, 0, 0);
    lineNumberArea->update();
}

int CodeEditor::lineNumberAreaWidth() {
    return calculateStandardWidth() + calculateAddressWidth();
}

void CodeEditor::updateLineNumberArea(const QRect& rect, int dy) {
    if (dy) {
        lineNumberArea->scroll(0, dy);
    }
    lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
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
    lineNumberArea->update();
}

int CodeEditor::calculateStandardWidth() const {
    if (!standardLineNumbering) {
        return 0;
    }
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    return fontMetrics().horizontalAdvance(QLatin1Char('9')) * (digits + 1) + 5;
}

int CodeEditor::calculateAddressWidth() const {
    if (!addressLineNumbering) {
        return 0;
    }
    return fontMetrics().horizontalAdvance(":0000") + 15;
}
