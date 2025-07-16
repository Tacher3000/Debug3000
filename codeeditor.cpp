#include "codeeditor.h"
#include <QRegularExpression>
#include <QTextBlock>
#include <QPainter>
#include <QFontMetrics>

const QString CodeEditor::ADDRESS_FORMAT = "CS:0000";

CodeEditor::SyntaxHighlighter::SyntaxHighlighter(QTextDocument* parent) : QSyntaxHighlighter(parent), enabled(true) {}

void CodeEditor::SyntaxHighlighter::setEnabled(bool enabled) {
    this->enabled = enabled;
    rehighlight();
}

void CodeEditor::SyntaxHighlighter::highlightBlock(const QString& text) {
    if (!enabled) return;

    QTextCharFormat instructionFormat;
    instructionFormat.setForeground(Qt::blue);
    QRegularExpression instructionRegex("\\b(mov|int|add|sub|cmp|jmp|je|jne|jz|jnz|jg|jge|jl|jle|mul|div|imul|idiv|inc|dec|push|pop|call|ret)\\b", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatchIterator it = instructionRegex.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        setFormat(match.capturedStart(), match.capturedLength(), instructionFormat);
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

CodeEditor::CodeEditor(QWidget* parent) : QPlainTextEdit(parent), standardLineNumbering(true), addressLineNumbering(true), currentLineHighlight(true), lineWrap(false), syntaxHighlighting(true), showMemoryDump(false) {
    lineNumberArea = new LineNumberArea(this);
    memoryDumpArea = new MemoryDumpArea(this);
    highlighter = new SyntaxHighlighter(document());
    setFont(QFont("Courier New", 10));
    setTabStopDistance(4 * fontMetrics().horizontalAdvance(' '));
    connect(this, &QPlainTextEdit::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest, this, &CodeEditor::updateLineNumberArea);
    connect(this, &QPlainTextEdit::updateRequest, this, &CodeEditor::updateMemoryDumpArea);
    connect(this, &QPlainTextEdit::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);
    connect(this, &QPlainTextEdit::textChanged, this, &CodeEditor::updateMemoryDump);
    updateLineNumberAreaWidth();
    highlightCurrentLine();
    lineNumberArea->setVisible(true);
    memoryDumpArea->setVisible(false);
}

CodeEditor::~CodeEditor() {
    delete lineNumberArea;
    delete memoryDumpArea;
    delete highlighter;
}

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

void CodeEditor::setShowMemoryDump(bool enabled, const QString& segment, const QString& offset, int lineCount) {
    showMemoryDump = enabled;
    memoryDumpSegment = segment;
    memoryDumpOffset = offset;
    memoryDumpLineCount = lineCount;
    memoryDumpArea->setVisible(enabled);
    updateLineNumberAreaWidth();
    memoryDumpArea->update();
}

void CodeEditor::setTheme(const QString& theme, const QColor& backgroundColor, const QColor& textColor, const QColor& highlightColor) {
    this->theme = theme;
    this->backgroundColor = backgroundColor;
    this->textColor = textColor;
    this->highlightColor = highlightColor;
    QPalette palette = this->palette();
    palette.setColor(QPalette::Base, backgroundColor);
    palette.setColor(QPalette::Text, textColor);
    setPalette(palette);
    highlightCurrentLine();
}

void CodeEditor::setFont(const QFont& font) {
    QPlainTextEdit::setFont(font);
    setTabStopDistance(4 * fontMetrics().horizontalAdvance(' '));
    updateLineNumberAreaWidth();
    lineNumberArea->update();
    memoryDumpArea->update();
}

void CodeEditor::setLineWrap(bool enabled) {
    setLineWrapMode(enabled ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
    lineWrap = enabled;
    lineNumberArea->update();
    memoryDumpArea->update();
}

void CodeEditor::setSyntaxHighlighting(bool enabled) {
    syntaxHighlighting = enabled;
    static_cast<SyntaxHighlighter*>(highlighter)->setEnabled(enabled);
}

QString CodeEditor::getText() const { return toPlainText(); }
void CodeEditor::setText(const QString& text) { setPlainText(text); }

int CodeEditor::calculateInstructionLength(const QString& text) {
    QString trimmed = text.trimmed().toUpper();
    if (trimmed.isEmpty() || trimmed.startsWith("A ") || trimmed.startsWith("E ")) return 0;

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

        if (trimmed.startsWith("E ", Qt::CaseInsensitive)) {
            processEditCommand(trimmed, currentBlockNumber);
        }

        if (addressLineNumbering) {
            if (trimmed.startsWith("A ", Qt::CaseInsensitive)) {
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
                    QString addressText = QString("CS:%1").arg(nextAddress, 4, 16, QChar('0')).toUpper();
                    painter.setPen(Qt::black);
                    painter.drawText(standardWidth + MARGIN_LEFT, top, addressWidth - MARGIN_RIGHT, fontMetrics().height(),
                                     Qt::AlignRight | Qt::AlignVCenter, addressText);
                }
                addressMode = false;
                showNextEmptyAddress = false;
            }
            else if (addressMode && !trimmed.isEmpty()) {
                if (currentBlockNumber >= blockNumber && currentBlock.isVisible() && bottom >= event->rect().top()) {
                    QString addressText = QString("CS:%1").arg(nextAddress, 4, 16, QChar('0')).toUpper();
                    painter.setPen(Qt::black);
                    painter.drawText(standardWidth + MARGIN_LEFT, top, addressWidth - MARGIN_RIGHT, fontMetrics().height(),
                                     Qt::AlignRight | Qt::AlignVCenter, addressText);
                }
                nextAddress += calculateInstructionLength(trimmed);
            }
        }

        if (standardLineNumbering && currentBlockNumber >= blockNumber && currentBlock.isVisible() && bottom >= event->rect().top()) {
            painter.setPen(Qt::black);
            const QString number = QString::number(currentBlockNumber + 1);
            painter.drawText(MARGIN_LEFT, top, standardWidth - MARGIN_RIGHT, fontMetrics().height(),
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

void CodeEditor::memoryDumpAreaPaintEvent(QPaintEvent* event) {
    if (!showMemoryDump) return;

    QPainter painter(memoryDumpArea);
    painter.fillRect(event->rect(), Qt::lightGray);

    int top = 0;
    bool ok;
    int offset = memoryDumpOffset.toInt(&ok, 16);
    if (!ok) offset = 0x200;

    int segment = memoryDumpSegment.toInt(&ok, 16);
    if (!ok) segment = 0x1000;

    for (int i = 0; i < memoryDumpLineCount; ++i) {
        QString dumpLine;
        int currentOffset = offset + i * 16;
        dumpLine += QString("%1:%2  ")
                        .arg(segment, 4, 16, QChar('0'))
                        .arg(currentOffset, 4, 16, QChar('0'))
                        .toUpper();

        QByteArray data = memoryChanges.value(currentOffset, QByteArray(16, 0));
        for (int j = 0; j < 16; ++j) {
            dumpLine += QString("%1 ").arg((unsigned char)data[j], 2, 16, QChar('0')).toUpper();
        }
        dumpLine += " ";
        for (int j = 0; j < 16; ++j) {
            unsigned char c = data[j];
            dumpLine += (c >= 32 && c <= 126) ? QChar(c) : QChar('.');
        }

        painter.setPen(Qt::black);
        painter.drawText(MARGIN_LEFT, top + i * fontMetrics().height(), memoryDumpAreaWidth() - MARGIN_RIGHT, fontMetrics().height(),
                         Qt::AlignLeft | Qt::AlignVCenter, dumpLine);
    }
}

void CodeEditor::resizeEvent(QResizeEvent* event) {
    QPlainTextEdit::resizeEvent(event);
    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
    memoryDumpArea->setGeometry(QRect(cr.right() - memoryDumpAreaWidth(), cr.top(), memoryDumpAreaWidth(), cr.height()));
    lineNumberArea->update();
    memoryDumpArea->update();
}

void CodeEditor::updateLineNumberAreaWidth() {
    int width = lineNumberAreaWidth();
    setViewportMargins(width, 0, showMemoryDump ? memoryDumpAreaWidth() : 0, 0);
    lineNumberArea->update();
    memoryDumpArea->update();
}

int CodeEditor::lineNumberAreaWidth() {
    return calculateStandardWidth() + calculateAddressWidth();
}

int CodeEditor::memoryDumpAreaWidth() {
    if (!showMemoryDump) return 0;
    return fontMetrics().horizontalAdvance("XXXX:XXXX  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................") + MARGIN_LEFT + MARGIN_RIGHT;
}

void CodeEditor::updateLineNumberArea(const QRect& rect, int dy) {
    if (dy) {
        lineNumberArea->scroll(0, dy);
    }
    lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
}

void CodeEditor::updateMemoryDumpArea(const QRect& rect, int dy) {
    if (dy) {
        memoryDumpArea->scroll(0, dy);
    }
    memoryDumpArea->update(0, rect.y(), memoryDumpArea->width(), rect.height());
}

void CodeEditor::highlightCurrentLine() {
    QList<QTextEdit::ExtraSelection> extraSelections;
    if (currentLineHighlight) {
        QTextEdit::ExtraSelection selection;
        selection.format.setBackground(highlightColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }
    setExtraSelections(extraSelections);
    lineNumberArea->update();
    memoryDumpArea->update();
}

void CodeEditor::updateMemoryDump() {
    memoryChanges.clear();
    QTextBlock block = document()->begin();
    int blockNumber = 0;
    while (block.isValid()) {
        QString line = block.text().trimmed();
        if (line.startsWith("E ", Qt::CaseInsensitive)) {
            processEditCommand(line, blockNumber);
        }
        block = block.next();
        ++blockNumber;
    }
    memoryDumpArea->update();
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
    return fontMetrics().horizontalAdvance(QLatin1Char('9')) * (digits + 1) + MARGIN_LEFT + MARGIN_RIGHT;
}

int CodeEditor::calculateAddressWidth() const {
    if (!addressLineNumbering) {
        return 0;
    }
    return fontMetrics().horizontalAdvance(ADDRESS_FORMAT) + ADDRESS_EXTRA_WIDTH;
}

void CodeEditor::processEditCommand(const QString& line, int blockNumber) {
    QString trimmed = line.trimmed();
    QRegularExpression re("^E\\s+([0-9A-Fa-f]+)\\s+(.+)$", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match = re.match(trimmed);
    if (!match.hasMatch()) return;

    bool ok;
    int address = match.captured(1).toInt(&ok, 16);
    if (!ok) return;

    QString dataPart = match.captured(2);
    QByteArray data;

    QRegularExpression stringRe("^\"(.+)\"$", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch stringMatch = stringRe.match(dataPart);
    if (stringMatch.hasMatch()) {
        QString str = stringMatch.captured(1);
        data = str.toLatin1();
    } else {
        QStringList byteParts = dataPart.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        for (const QString& byteStr : byteParts) {
            int byte = byteStr.toInt(&ok, 16);
            if (ok && byte >= 0 && byte <= 255) {
                data.append(static_cast<char>(byte));
            }
        }
    }

    if (!data.isEmpty()) {
        int currentAddress = address;
        int dataSize = data.size();
        int pos = 0;

        while (pos < dataSize) {
            QByteArray block;
            int bytesToTake = qMin(16, dataSize - pos);
            block = data.mid(pos, bytesToTake);
            if (block.size() < 16) {
                block.resize(16, 0);
            }
            memoryChanges[currentAddress] = block;
            pos += 16;
            currentAddress += 16;
        }
    }
}
