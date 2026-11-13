// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dterminalscreen.h"

#include <QDebug>

#include <algorithm>

// 终端屏幕模型说明：
// - 屏幕图像按行存储（每行变长，缩列后旧内容不截断，扩列后按需补空白）；
// - 图像内偏移 = 行号 * 列数 + 列号（与列数绑定的逻辑坐标，仅同一尺寸周期内有效）；
// - 宽字符占用两个单元格：主单元存码点，后随单元码点为 0（渲染时视为延续格）。

DTerminalScreen::DTerminalScreen(int lines, int columns)
    : m_lineCount(lines)
    , m_columnCount(columns)
    , m_lines(lines + 1)
    , m_lineFlags(lines + 1, LineDefault)
    , m_history(DTerminalHistory::create(DTerminalHistory::StorageKind::None))
{
    m_urlExtractor.setScreen(this);
    initTabStops();
    clearSelection();
    reset();
}

DTerminalScreen::~DTerminalScreen()
{
    delete m_history;
}

DTerminalChar DTerminalScreen::defaultChar()
{
    return DTerminalChar();
}

// ---- 光标移动 ----

void DTerminalScreen::cursorUp(int n)
{
    if (n < 1)
        n = 1;  // 缺省
    const int stop = m_cursorRow < m_topMargin ? 0 : m_topMargin;
    m_cursorRow = qMax(stop, m_cursorRow - n);
}

void DTerminalScreen::cursorDown(int n)
{
    if (n < 1)
        n = 1;
    if (n > kMaxScreenArgument)
        n = kMaxScreenArgument;
    const int stop = m_cursorRow > m_bottomMargin ? m_lineCount - 1 : m_bottomMargin;
    m_cursorRow = qMin(stop, m_cursorRow + n);
}

void DTerminalScreen::cursorLeft(int n)
{
    if (n < 1)
        n = 1;
    m_cursorColumn = qMax(0, m_cursorColumn - n);
}

void DTerminalScreen::cursorRight(int n)
{
    if (n < 1)
        n = 1;
    if (n > kMaxScreenArgument)
        n = kMaxScreenArgument;
    m_cursorColumn = qMin(screenLineColumns(m_cursorRow) - 1, m_cursorColumn + n);
}

void DTerminalScreen::setMargins(int top, int bottom)
{
    if (top == 0)
        top = 1;              // 缺省
    if (bottom == 0)
        bottom = m_lineCount; // 缺省
    top -= 1;                 // 转内部行号
    bottom -= 1;

    if (!(0 <= top && top < bottom && bottom < m_lineCount))
        return;               // 越界忽略

    m_topMargin = top;
    m_bottomMargin = bottom;
    m_cursorColumn = 0;
    m_cursorRow = getMode(ModeOrigin) ? top : 0;
}

void DTerminalScreen::setDefaultMargins()
{
    m_topMargin = 0;
    m_bottomMargin = m_lineCount - 1;
}

void DTerminalScreen::index()
{
    if (m_cursorRow == m_bottomMargin)
        scrollUp(1);
    else if (m_cursorRow < m_lineCount - 1)
        m_cursorRow += 1;
}

void DTerminalScreen::reverseIndex()
{
    if (m_cursorRow == m_topMargin)
        scrollDownFrom(m_topMargin, 1);
    else if (m_cursorRow > 0)
        m_cursorRow -= 1;
}

void DTerminalScreen::nextLine()
{
    toStartOfLine();
    index();
}

void DTerminalScreen::newLine()
{
    if (getMode(ModeNewLine))
        toStartOfLine();
    index();
}

// ---- 编辑 ----

void DTerminalScreen::eraseChars(int n)
{
    if (n < 1)
        n = 1;
    if (n > kMaxScreenArgument)
        n = kMaxScreenArgument;
    const int end = qBound(0, m_cursorColumn + n - 1, m_columnCount - 1);
    clearRange(cellIndex(m_cursorColumn, m_cursorRow), cellIndex(end, m_cursorRow), ' ', false);
}

void DTerminalScreen::deleteChars(int n)
{
    Q_ASSERT(n >= 0);

    if (n == 0)
        n = 1;  // 至少删一个

    // 光标已越过行尾则无事可做
    if (m_cursorColumn >= m_lines[m_cursorRow].count())
        return;

    if (m_cursorColumn + n > m_lines[m_cursorRow].count())
        n = m_lines[m_cursorRow].count() - m_cursorColumn;

    Q_ASSERT(n >= 0);
    Q_ASSERT(m_cursorColumn + n <= m_lines[m_cursorRow].count());

    m_lines[m_cursorRow].remove(m_cursorColumn, n);
}

void DTerminalScreen::insertChars(int n)
{
    if (n == 0)
        n = 1;

    if (m_lines[m_cursorRow].size() < m_cursorColumn)
        m_lines[m_cursorRow].resize(m_cursorColumn);

    m_lines[m_cursorRow].insert(m_cursorColumn, n, DTerminalChar());

    if (m_lines[m_cursorRow].count() > m_columnCount)
        m_lines[m_cursorRow].resize(m_columnCount);
}

void DTerminalScreen::repeatChars(int count)
{
    if (count == 0)
        count = 1;
    for (int i = 0; i < count; ++i)
        displayCharacter(m_lastDrawnChar);
}

void DTerminalScreen::deleteLines(int n)
{
    if (n == 0)
        n = 1;
    scrollUpFrom(m_cursorRow, n);
}

void DTerminalScreen::insertLines(int n)
{
    if (n == 0)
        n = 1;
    scrollDownFrom(m_cursorRow, n);
}

// ---- 模式 ----

void DTerminalScreen::setMode(DTerminalScreenMode mode)
{
    m_modes[mode] = true;
    if (mode == ModeOrigin) {
        m_cursorColumn = 0;
        m_cursorRow = m_topMargin;
    }
}

void DTerminalScreen::resetMode(DTerminalScreenMode mode)
{
    m_modes[mode] = false;
    if (mode == ModeOrigin) {
        m_cursorColumn = 0;
        m_cursorRow = 0;
    }
}

void DTerminalScreen::saveMode(DTerminalScreenMode mode)
{
    m_savedModes[mode] = m_modes[mode];
}

void DTerminalScreen::restoreMode(DTerminalScreenMode mode)
{
    m_modes[mode] = m_savedModes[mode];
}

bool DTerminalScreen::getMode(DTerminalScreenMode mode) const
{
    return m_modes[mode];
}

// ---- 光标保存/恢复 ----

void DTerminalScreen::saveCursor()
{
    m_savedState.cursorColumn = m_cursorColumn;
    m_savedState.cursorRow = m_cursorRow;
    m_savedState.style = m_currentStyle;
    m_savedState.fore = m_currentFore;
    m_savedState.back = m_currentBack;
}

void DTerminalScreen::restoreCursor()
{
    m_cursorColumn = qMin(m_savedState.cursorColumn, m_columnCount - 1);
    m_cursorRow = qMin(m_savedState.cursorRow, m_lineCount - 1);
    m_currentStyle = m_savedState.style;
    m_currentFore = m_savedState.fore;
    m_currentBack = m_savedState.back;
    updateEffectiveStyle();
}

// ---- 清屏 ----

void DTerminalScreen::clearToEndOfScreen()
{
    clearRange(cellIndex(m_cursorColumn, m_cursorRow),
               cellIndex(m_columnCount - 1, m_lineCount - 1), ' ');
}

void DTerminalScreen::clearToBeginOfScreen()
{
    clearRange(cellIndex(0, 0), cellIndex(m_cursorColumn, m_cursorRow), ' ');
}

void DTerminalScreen::clearEntireScreen()
{
    clearRange(cellIndex(0, 0), cellIndex(m_columnCount - 1, m_lineCount - 1), ' ');
}

void DTerminalScreen::helpAlign()
{
    clearRange(cellIndex(0, 0), cellIndex(m_columnCount - 1, m_lineCount - 1), 'E');
}

void DTerminalScreen::clearToEndOfLine()
{
    clearRange(cellIndex(m_cursorColumn, m_cursorRow),
               cellIndex(m_columnCount - 1, m_cursorRow), ' ');
}

void DTerminalScreen::clearToBeginOfLine()
{
    clearRange(cellIndex(0, m_cursorRow), cellIndex(m_cursorColumn, m_cursorRow), ' ');
}

void DTerminalScreen::clearEntireLine()
{
    clearRange(cellIndex(0, m_cursorRow), cellIndex(m_columnCount - 1, m_cursorRow), ' ');
}

void DTerminalScreen::clear()
{
    clearEntireScreen();
    home();
}

void DTerminalScreen::home()
{
    m_cursorColumn = 0;
    m_cursorRow = 0;
}

void DTerminalScreen::reset(bool clearScreen)
{
    setMode(ModeWrap);
    saveMode(ModeWrap);      // 列尾换行
    resetMode(ModeOrigin);
    saveMode(ModeOrigin);    // 绝对定位
    resetMode(ModeInsert);
    saveMode(ModeInsert);    // 覆写
    setMode(ModeShowCursor); // 光标可见
    resetMode(ModeInverseVideo);
    resetMode(ModeNewLine);

    m_topMargin = 0;
    m_bottomMargin = m_lineCount - 1;

    setDefaultRendition();
    saveCursor();

    if (clearScreen)
        clear();
}

// ---- 外观 ----

void DTerminalScreen::swapColors(DTerminalChar &cell) const
{
    const DTerminalCellColor fore = cell.fg;
    cell.fg = cell.bg;
    cell.bg = fore;
}

void DTerminalScreen::updateEffectiveStyle()
{
    m_effectiveStyle = m_currentStyle;
    if (m_currentStyle & StyleReverse) {
        m_effectiveFore = m_currentBack;
        m_effectiveBack = m_currentFore;
    } else {
        m_effectiveFore = m_currentFore;
        m_effectiveBack = m_currentBack;
    }

    if (m_currentStyle & StyleBold)
        m_effectiveFore.setIntensive();
}

void DTerminalScreen::setRendition(quint16 rendition)
{
    m_currentStyle |= rendition;
    updateEffectiveStyle();
}

void DTerminalScreen::resetRendition(quint16 rendition)
{
    m_currentStyle &= ~rendition;
    updateEffectiveStyle();
}

void DTerminalScreen::setDefaultRendition()
{
    setForeColor(ColorSpaceDefault, kDefaultForeIndex);
    setBackColor(ColorSpaceDefault, kDefaultBackIndex);
    m_currentStyle = StylePlain;
    updateEffectiveStyle();
}

void DTerminalScreen::setForeColor(int space, int color)
{
    m_currentFore = DTerminalCellColor(space, color);
    if (m_currentFore.isValid())
        updateEffectiveStyle();
    else
        setForeColor(ColorSpaceDefault, kDefaultForeIndex);
}

void DTerminalScreen::setBackColor(int space, int color)
{
    m_currentBack = DTerminalCellColor(space, color);
    if (m_currentBack.isValid())
        updateEffectiveStyle();
    else
        setBackColor(ColorSpaceDefault, kDefaultBackIndex);
}

// ---- 光标定位 ----

void DTerminalScreen::setCursorPosition(int row, int column)
{
    setCursorRow(row);
    setCursorColumn(column);
}

void DTerminalScreen::setCursorColumn(int column)
{
    if (column == 0)
        column = 1;  // 缺省
    column -= 1;     // 转内部列号
    m_cursorColumn = qMax(0, qMin(m_columnCount - 1, column));
}

void DTerminalScreen::setCursorRow(int row)
{
    if (row < 1)
        row = 1;
    if (row > kMaxScreenArgument)
        row = kMaxScreenArgument;
    row += getMode(ModeOrigin) ? m_topMargin : 0;
    m_cursorRow = qBound(0, row - 1, m_lineCount - 1);
}

void DTerminalScreen::toStartOfLine()
{
    m_cursorColumn = 0;
}

void DTerminalScreen::backspace()
{
    m_cursorColumn = qMin(m_columnCount - 1, m_cursorColumn);  // 不换行
    m_cursorColumn = qMax(0, m_cursorColumn - 1);

    if (m_lines[m_cursorRow].size() < m_cursorColumn + 1)
        m_lines[m_cursorRow].resize(m_cursorColumn + 1);
}

void DTerminalScreen::tab(int n)
{
    if (n == 0)
        n = 1;
    while (n > 0 && m_cursorColumn < m_columnCount - 1) {
        cursorRight(1);
        while (m_cursorColumn < m_columnCount - 1 && !m_tabStops[m_cursorColumn]) {
            const int oldColumn = m_cursorColumn;
            cursorRight(1);
            if (oldColumn == m_cursorColumn)
                break;
        }
        n--;
    }
}

void DTerminalScreen::backtab(int n)
{
    if (n == 0)
        n = 1;
    while (n > 0 && m_cursorColumn > 0) {
        cursorLeft(1);
        while (m_cursorColumn > 0 && !m_tabStops[m_cursorColumn])
            cursorLeft(1);
        n--;
    }
}

void DTerminalScreen::clearTabStops()
{
    for (int i = 0; i < m_columnCount; ++i)
        m_tabStops[i] = false;
}

void DTerminalScreen::changeTabStop(bool set)
{
    if (m_cursorColumn >= m_columnCount)
        return;
    m_tabStops[m_cursorColumn] = set;
}

void DTerminalScreen::initTabStops()
{
    m_tabStops.resize(m_columnCount);
    // 首个制表位多一格（从 1 起算的惯例）
    for (int i = 0; i < m_columnCount; ++i)
        m_tabStops[i] = (i % 8 == 0 && i != 0);
}

// ---- 显示字符 ----

void DTerminalScreen::checkSelection(int from, int to)
{
    if (m_selectionAnchor == -1)
        return;
    const int screenTop = cellIndex(0, m_history->lineCount());
    if (m_selectionBottomRight >= (from + screenTop) && m_selectionTopLeft <= (to + screenTop))
        clearSelection();
}

void DTerminalScreen::displayCharacter(uint c)
{
    // VT100 先换行再落字符
    int width = DCharWidth::cellWidth(c);
    if (width <= 0)
        return;

    if (m_cursorColumn + width > m_columnCount) {
        if (getMode(ModeWrap)) {
            m_lineFlags[m_cursorRow] = m_lineFlags[m_cursorRow] | LineWrapped;
            nextLine();
        } else {
            m_cursorColumn = m_columnCount - width;
        }
    }

    if (m_cursorRow >= m_lines.size())
        return;

    int size = m_lines[m_cursorRow].size();
    if (size < m_cursorColumn + width)
        m_lines[m_cursorRow].resize(m_cursorColumn + width);

    if (getMode(ModeInsert))
        insertChars(width);

    m_lastPos = cellIndex(m_cursorColumn, m_cursorRow);

    // 选区被覆盖则清除
    checkSelection(m_lastPos, m_lastPos);

    DTerminalChar &currentChar = m_lines[m_cursorRow][m_cursorColumn];
    currentChar.codePoint = c;
    currentChar.fg = m_effectiveFore;
    currentChar.bg = m_effectiveBack;
    currentChar.style = m_effectiveStyle;

    m_lastDrawnChar = c;

    // 宽字符的后续单元以码点 0 占位
    int fill = width - 1;
    int offset = 1;
    while (fill > 0) {
        if (m_lines[m_cursorRow].size() < m_cursorColumn + offset + 1)
            m_lines[m_cursorRow].resize(m_cursorColumn + offset + 1);

        DTerminalChar &trail = m_lines[m_cursorRow][m_cursorColumn + offset];
        trail.codePoint = 0;
        trail.fg = m_effectiveFore;
        trail.bg = m_effectiveBack;
        trail.style = m_effectiveStyle;

        offset++;
        fill--;
    }
    m_cursorColumn += width;

    if (c <= 0xFFFF) {
        m_urlExtractor.appendUrlText(QChar(static_cast<ushort>(c)));
    } else {
        // 超平面字符拆为代理对（deepin 自有修正移植）
        m_urlExtractor.appendUrlText(QChar::highSurrogate(c));
        m_urlExtractor.appendUrlText(QChar::lowSurrogate(c));
    }
}

// ---- 滚动 ----

void DTerminalScreen::scrollUp(int n)
{
    if (n < 1)
        n = 1;
    if (m_topMargin == 0)
        pushHistoryLine();  // 移入历史
    scrollUpFrom(m_topMargin, n);
}

void DTerminalScreen::scrollUpFrom(int from, int n)
{
    if (n <= 0)
        return;
    if (from > m_bottomMargin)
        return;
    if (from + n > m_bottomMargin)
        n = m_bottomMargin + 1 - from;

    m_scrolledLines -= n;
    m_lastScrolledRegion = QRect(0, m_topMargin, m_columnCount - 1, m_bottomMargin - m_topMargin);

    moveRange(cellIndex(0, from), cellIndex(0, from + n), cellIndex(m_columnCount, m_bottomMargin));
    clearRange(cellIndex(0, m_bottomMargin - n + 1), cellIndex(m_columnCount - 1, m_bottomMargin), ' ');
}

void DTerminalScreen::scrollDown(int n)
{
    if (n == 0)
        n = 1;
    scrollDownFrom(m_topMargin, n);
}

void DTerminalScreen::scrollDownFrom(int from, int n)
{
    m_scrolledLines += n;

    if (n <= 0)
        return;
    if (from > m_bottomMargin)
        return;
    if (from + n > m_bottomMargin)
        n = m_bottomMargin - from;
    moveRange(cellIndex(0, from + n), cellIndex(0, from), cellIndex(m_columnCount - 1, m_bottomMargin - n));
    clearRange(cellIndex(0, from), cellIndex(m_columnCount - 1, from + n - 1), ' ');
}

// ---- 选区 ----

void DTerminalScreen::clearSelection()
{
    m_selectionBottomRight = -1;
    m_selectionTopLeft = -1;
    m_selectionAnchor = -1;
}

void DTerminalScreen::getSelectionStart(int &column, int &line) const
{
    if (m_selectionTopLeft != -1) {
        column = m_selectionTopLeft % m_columnCount;
        line = m_selectionTopLeft / m_columnCount;
    } else {
        column = m_cursorColumn + historyLineCount();
        line = m_cursorRow + historyLineCount();
    }
}

void DTerminalScreen::getSelectionEnd(int &column, int &line) const
{
    if (m_selectionBottomRight != -1) {
        column = m_selectionBottomRight % m_columnCount;
        line = m_selectionBottomRight / m_columnCount;
    } else {
        column = m_cursorColumn + historyLineCount();
        line = m_cursorRow + historyLineCount();
    }
}

void DTerminalScreen::setSelectionStart(int column, int line, bool blockSelectionMode)
{
    m_selectionAnchor = cellIndex(column, line);
    // 列号越界右端时回退一格
    if (column == m_columnCount)
        m_selectionAnchor--;

    m_selectionBottomRight = m_selectionAnchor;
    m_selectionTopLeft = m_selectionAnchor;
    m_blockSelection = blockSelectionMode;
}

void DTerminalScreen::setSelectionEnd(int column, int line)
{
    if (m_selectionAnchor == -1)
        return;

    int endPos = cellIndex(column, line);

    if (endPos < m_selectionAnchor) {
        m_selectionTopLeft = endPos;
        m_selectionBottomRight = m_selectionAnchor;
    } else {
        // 列号越界右端时回退一格
        if (column == m_columnCount)
            endPos--;

        m_selectionTopLeft = m_selectionAnchor;
        m_selectionBottomRight = endPos;
    }

    // 列块模式按列归一化
    if (m_blockSelection) {
        const int topRow = m_selectionTopLeft / m_columnCount;
        const int topColumn = m_selectionTopLeft % m_columnCount;
        const int bottomRow = m_selectionBottomRight / m_columnCount;
        const int bottomColumn = m_selectionBottomRight % m_columnCount;

        m_selectionTopLeft = cellIndex(qMin(topColumn, bottomColumn), topRow);
        m_selectionBottomRight = cellIndex(qMax(topColumn, bottomColumn), bottomRow);
    }
}

void DTerminalScreen::setSelectionAll()
{
    m_selectionAnchor = 0;
    m_selectionTopLeft = 0;
    m_selectionBottomRight = (historyLineCount() + m_cursorRow + 1) * m_columnCount - 1;
}

bool DTerminalScreen::isSelected(int column, int line) const
{
    bool columnInSelection = true;
    if (m_blockSelection) {
        columnInSelection = column >= (m_selectionTopLeft % m_columnCount)
                && column <= (m_selectionBottomRight % m_columnCount);
    }

    const int pos = cellIndex(column, line);
    return pos >= m_selectionTopLeft && pos <= m_selectionBottomRight && columnInSelection;
}

bool DTerminalScreen::isSelectionValid() const
{
    return m_selectionTopLeft >= 0 && m_selectionBottomRight >= 0;
}

QString DTerminalScreen::selectedText(TextOptions options) const
{
    if (!isSelectionValid())
        return QString();

    QString result;
    writeRange(result, m_selectionTopLeft, m_selectionBottomRight, options);
    return result;
}

QString DTerminalScreen::text(int startIndex, int endIndex, TextOptions options) const
{
    QString result;
    writeRange(result, startIndex, endIndex, options);
    return result;
}

// ---- 文本提取 ----

int DTerminalScreen::lineLength(int line) const
{
    // 历史行返回实际行长；屏幕行按满列计
    const bool isInHistoryBuffer = line < m_history->lineCount();
    if (isInHistoryBuffer)
        return m_history->lineLength(line);
    return m_columnCount;
}

DTerminalChar *DTerminalScreen::characterBuffer(int size)
{
    // 静态缓冲避免反复构造元素
    static const int kMaxChars = 1024;
    static QVector<DTerminalChar> buffer(kMaxChars);

    if (buffer.count() < size) {
        buffer.resize(size);
        if (buffer.count() < size)
            qCritical() << "DTerminalScreen: cannot resize character buffer to" << size;
    }
    return buffer.data();
}

void DTerminalScreen::writeRange(QString &out, int startIndex, int endIndex, TextOptions options) const
{
    const int top = startIndex / m_columnCount;
    const int left = startIndex % m_columnCount;

    const int bottom = endIndex / m_columnCount;
    const int right = endIndex % m_columnCount;

    Q_ASSERT(top >= 0 && left >= 0 && bottom >= 0 && right >= 0);

    for (int y = top; y <= bottom; ++y) {
        int start = 0;
        if (y == top || m_blockSelection)
            start = left;

        int count = -1;
        if (y == bottom || m_blockSelection)
            count = right - start + 1;

        const bool appendNewLine = (y != bottom);
        const int copied = copyLineToText(out, y, start, count, appendNewLine, options);

        // 选区越过行尾时补一个换行符
        if (y == bottom && copied < count) {
            out += QLatin1Char('\n');
        }
    }
}

int DTerminalScreen::copyLineToText(QString &out, int line, int start, int count,
                                    bool appendNewLine, TextOptions options) const
{
    const int lineLen = lineLength(line);
    // 允许在行尾追加换行/空格，缓冲多留一个元素
    DTerminalChar *buffer = characterBuffer((count > -1 ? count : lineLen - start) + 1);
    quint8 currentLineFlags = 0;

    // 历史行：无尾随空白，无需裁剪
    if (line < m_history->lineCount()) {
        start = qBound(0, start, lineLen - 1);

        if (count == -1)
            count = lineLen - start;
        else
            count = qMin(start + count, lineLen) - start;

        Q_ASSERT(start >= 0);
        Q_ASSERT(count >= 0);
        Q_ASSERT(start + count <= m_history->lineLength(line));

        m_history->lineCells(line, start, count, buffer);

        if (m_history->isWrappedLine(line))
            currentLineFlags |= LineWrapped;
    } else {
        if (count == -1)
            count = lineLen - start;
        Q_ASSERT(count >= 0);

        int screenLine = line - m_history->lineCount();
        Q_ASSERT(screenLine <= m_lineCount);
        screenLine = qMin(screenLine, m_lineCount);

        const DTerminalChar *data = m_lines[screenLine].constData();
        int length = m_lines[screenLine].count();

        // 非软换行行按需裁掉行尾空白
        if (options.testFlag(TextTrimTrailing) && ((m_lineFlags[screenLine] & LineWrapped) == 0)) {
            while (length > 0 && QChar(data[length - 1].codePoint).isSpace())
                length--;
        }

        if (start >= length) {
            count = 0;
        } else {
            const int end = qMin(start + count, length);
            if (start < end)
                std::copy(data + start, data + end, buffer);
            count = qBound(0, count, length - start);
        }

        Q_ASSERT(screenLine < m_lineFlags.count());
        currentLineFlags |= m_lineFlags[screenLine];
    }

    if (appendNewLine) {
        if ((currentLineFlags & LineWrapped) != 0) {
            // 软换行行不加分隔
        } else {
            buffer[count] = options.testFlag(TextKeepBreaks) ? DTerminalChar('\n', StylePlain,
                                                                             DTerminalCellColor(), DTerminalCellColor())
                                                             : DTerminalChar(' ', StylePlain,
                                                                             DTerminalCellColor(), DTerminalCellColor());
            count++;
        }
    }

    if (options.testFlag(TextTrimLeading)) {
        int spacesCount = 0;
        for (; spacesCount < count; ++spacesCount) {
            if (QChar::category(buffer[spacesCount].codePoint) != QChar::Category::Separator_Space)
                break;
        }

        if (spacesCount >= count)
            return 0;

        std::copy(buffer + spacesCount, buffer + count, buffer);
        count -= spacesCount;
    }

    // 输出文本：跳过宽字符延续格（码点 0），展开组合序列
    for (int i = 0; i < count; ++i) {
        const DTerminalChar &cell = buffer[i];
        if (cell.codePoint == 0)
            continue;
        if (cell.style & StyleExtended) {
            if (m_sequenceResolver)
                out += m_sequenceResolver(cell.codePoint);
            else
                out += QChar(cell.codePoint);
        } else {
            out += QChar(cell.codePoint);
        }
    }

    return count;
}

// ---- 图像快照 ----

void DTerminalScreen::copyFromHistory(DTerminalChar *dest, int startLine, int count) const
{
    Q_ASSERT(startLine >= 0 && count > 0 && startLine + count <= m_history->lineCount());

    for (int line = startLine; line < startLine + count; ++line) {
        const int length = qMin(m_columnCount, m_history->lineLength(line));
        const int destLineOffset = (line - startLine) * m_columnCount;

        m_history->lineCells(line, 0, length, dest + destLineOffset);

        for (int column = length; column < m_columnCount; ++column)
            dest[destLineOffset + column] = defaultChar();

        // 反色选区
        if (m_selectionAnchor != -1) {
            for (int column = 0; column < m_columnCount; ++column) {
                if (isSelected(column, line))
                    swapColors(dest[destLineOffset + column]);
            }
        }
    }
}

void DTerminalScreen::copyFromScreen(DTerminalChar *dest, int startLine, int count) const
{
    Q_ASSERT(startLine >= 0 && count > 0 && startLine + count <= m_lineCount);

    for (int line = startLine; line < startLine + count; ++line) {
        const int destLineStartIndex = (line - startLine) * m_columnCount;

        for (int column = 0; column < m_columnCount; ++column) {
            DTerminalChar &target = dest[destLineStartIndex + column];
            target = m_lines[line].value(column, defaultChar());

            // 反色选区
            if (m_selectionAnchor != -1 && isSelected(column, line + m_history->lineCount()))
                swapColors(target);
        }
    }
}

void DTerminalScreen::getImage(DTerminalChar *dest, int size, int startLine, int endLine) const
{
    Q_ASSERT(startLine >= 0);
    Q_ASSERT(endLine >= startLine && endLine < m_history->lineCount() + m_lineCount);

    const int mergedLines = endLine - startLine + 1;
    Q_ASSERT(size >= mergedLines * m_columnCount);
    Q_UNUSED(size);

    const int linesInHistoryBuffer = qBound(0, m_history->lineCount() - startLine, mergedLines);
    const int linesInScreenBuffer = mergedLines - linesInHistoryBuffer;

    if (linesInHistoryBuffer > 0)
        copyFromHistory(dest, startLine, linesInHistoryBuffer);

    if (linesInScreenBuffer > 0)
        copyFromScreen(dest + linesInHistoryBuffer * m_columnCount,
                       startLine + linesInHistoryBuffer - m_history->lineCount(),
                       linesInScreenBuffer);

    // 反显模式
    if (getMode(ModeInverseVideo)) {
        for (int i = 0; i < mergedLines * m_columnCount; ++i)
            swapColors(dest[i]);
    }

    // 标记光标位置
    const int cursorIndex = cellIndex(m_cursorColumn, m_cursorRow + linesInHistoryBuffer);
    if (getMode(ModeShowCursor) && cursorIndex < m_columnCount * mergedLines)
        dest[cursorIndex].style |= StyleCursor;
}

QVector<quint8> DTerminalScreen::getLineFlags(int startLine, int endLine) const
{
    Q_ASSERT(startLine >= 0);
    Q_ASSERT(endLine >= startLine && endLine < m_history->lineCount() + m_lineCount);

    const int mergedLines = endLine - startLine + 1;
    const int linesInHistory = qBound(0, m_history->lineCount() - startLine, mergedLines);
    const int linesInScreen = mergedLines - linesInHistory;

    QVector<quint8> result(mergedLines);
    int index = 0;

    for (int line = startLine; line < startLine + linesInHistory; ++line) {
        result[index] = m_history->lineFlags(line);
        index++;
    }

    const int firstScreenLine = startLine + linesInHistory - m_history->lineCount();
    for (int line = firstScreenLine; line < firstScreenLine + linesInScreen; ++line) {
        result[index] = m_lineFlags[line];
        index++;
    }

    return result;
}

int DTerminalScreen::screenLineColumns(int line) const
{
    const bool doubleWidthLine = m_lineFlags[line] & LineDoubleWidth;
    if (doubleWidthLine)
        return m_columnCount / 2;
    return m_columnCount;
}

// ---- 历史 ----

int DTerminalScreen::historyLineCount() const
{
    return m_history->lineCount();
}

void DTerminalScreen::setHistoryStorage(DTerminalHistory::StorageKind kind, int maxLines, bool copyContent)
{
    clearSelection();

    m_historyKind = kind;
    m_historyMaxLines = maxLines;

    DTerminalHistory *oldHistory = m_history;
    DTerminalHistory *newHistory = DTerminalHistory::create(kind, maxLines);
    if (copyContent && oldHistory)
        DTerminalHistory::migrate(oldHistory, newHistory);

    m_history = newHistory;
    delete oldHistory;
}

void DTerminalScreen::clearHistoryContent()
{
    setHistoryStorage(m_historyKind, m_historyMaxLines, false);
}

bool DTerminalScreen::hasHistory() const
{
    return m_history->hasScrolledLines();
}

void DTerminalScreen::pushFirstHistoryLine()
{
    const bool removeLine = m_history->lineCount() == m_history->maxLineCount();
    m_history->appendCells(m_lines[0].constData(), m_lines[0].count());
    m_history->appendLine(m_lineFlags[0]);

    // 历史满时顶掉一行，需同步移除越界 URL
    if (removeLine)
        m_urlExtractor.historyLinesRemoved(1);

    m_lines.pop_front();
    m_lineFlags.remove(0);
}

void DTerminalScreen::pushHistoryLine()
{
    if (!hasHistory())
        return;

    const int oldHistLines = m_history->lineCount();

    m_history->appendCells(m_lines[0].constData(), m_lines[0].count());
    m_history->appendLine(static_cast<quint8>(m_lineFlags[0] & LineWrapped));

    const int newHistLines = m_history->lineCount();

    const bool beginIsTL = (m_selectionAnchor == m_selectionTopLeft);

    // 历史已满时累计丢弃行数
    if (newHistLines == oldHistLines)
        m_droppedLines++;

    // 选区随滚动调整
    if (newHistLines > oldHistLines) {
        if (m_selectionAnchor != -1) {
            m_selectionTopLeft += m_columnCount;
            m_selectionBottomRight += m_columnCount;
        }
    }

    if (m_selectionAnchor != -1) {
        const int top_BR = cellIndex(0, 1 + newHistLines);

        if (m_selectionTopLeft < top_BR)
            m_selectionTopLeft -= m_columnCount;

        if (m_selectionBottomRight < top_BR)
            m_selectionBottomRight -= m_columnCount;

        if (m_selectionBottomRight < 0) {
            clearSelection();
        } else {
            if (m_selectionTopLeft < 0)
                m_selectionTopLeft = 0;
        }

        if (beginIsTL)
            m_selectionAnchor = m_selectionTopLeft;
        else
            m_selectionAnchor = m_selectionBottomRight;
    }
}

void DTerminalScreen::setLineFlag(quint8 flag, bool enable)
{
    if (enable)
        m_lineFlags[m_cursorRow] = m_lineFlags[m_cursorRow] | flag;
    else
        m_lineFlags[m_cursorRow] = m_lineFlags[m_cursorRow] & ~flag;
}

// ---- 尺寸变更 ----

int DTerminalScreen::cursorLine() const
{
    if (m_modes[ModeAlternate])
        return m_savedState.cursorRow;
    return m_cursorRow;
}

void DTerminalScreen::setCursorLine(int newLine)
{
    if (m_modes[ModeAlternate])
        m_savedState.cursorRow = newLine;
    else
        m_cursorRow = newLine;
}

void DTerminalScreen::resizeImage(int newLines, int newColumns)
{
    if (newLines == m_lineCount && newColumns == m_columnCount)
        return;

    // 记录滚动位置，修复滚动毛刺
    m_oldTotalLines = m_lineCount + historyLineCount();
    m_resizePending = true;

    int cursorLine = this->cursorLine();
    const int oldCursorLine = (cursorLine == m_lineCount - 1 || cursorLine > newLines - 1)
            ? newLines - 1 : cursorLine;

    // 历史回绕：把屏幕上的软换行行并入历史后统一回绕
    if (m_reflowEnabled && newColumns != m_columnCount
            && m_history->lineCount() && m_history->maxLineCount()) {
        while (m_history->isWrappedLine(m_history->lineCount() - 1)) {
            pushFirstHistoryLine();
            cursorLine--;
        }
        const int removedLines = m_history->reflow(newColumns);

        if (removedLines)
            m_urlExtractor.historyLinesRemoved(removedLines);
    }

    // 屏幕内回绕
    if (m_reflowEnabled && newColumns != m_columnCount) {
        int cursorLineCorrection = 0;
        if (m_view && m_foregroundNameProvider) {
            // zsh 与其他 shell 输入行处理不同，需要识别并校正命令行列号
            const QString foregroundName = m_foregroundNameProvider(m_sessionId);
            if (foregroundName == QLatin1String("zsh")
                    && cursorLine > 0
                    && ((m_lineFlags[cursorLine - 1] & LineWrapped) != 0)) {
                while (cursorLine + cursorLineCorrection > 0
                       && (m_lineFlags[cursorLine + cursorLineCorrection - 1] & LineWrapped) != 0) {
                    cursorLineCorrection--;
                }
            }
        }

        int currentPos = 0;
        while (currentPos < cursorLine + cursorLineCorrection && currentPos < m_lines.count() - 1) {
            // 合并软换行行
            if ((m_lineFlags[currentPos] & LineWrapped) != 0) {
                m_lines[currentPos].append(std::move(m_lines[currentPos + 1]));
                m_lines.remove(currentPos + 1);
                m_lineFlags.remove(currentPos);
                cursorLine--;
                continue;
            }

            // 忽略行尾空白
            int lineSize = m_lines[currentPos].size();
            while (lineSize > 0 && QChar(m_lines[currentPos][lineSize - 1].codePoint).isSpace())
                lineSize--;

            // 超出新列数则拆行下移
            if (lineSize > newColumns) {
                QVector<DTerminalChar> values = m_lines[currentPos].mid(newColumns);
                m_lines[currentPos].resize(newColumns);
                m_lineFlags.insert(currentPos + 1, m_lineFlags[currentPos]);
                m_lines.insert(currentPos + 1, std::move(values));
                m_lineFlags[currentPos] |= LineWrapped;
                cursorLine++;
            }
            currentPos += 1;
        }
    }

    // 光标行超出新行数时把屏幕顶行压入历史
    while (cursorLine > newLines - 1) {
        pushFirstHistoryLine();
        cursorLine--;
    }

    // 光标行不足时从历史取回
    if (m_reflowEnabled) {
        QVector<DTerminalChar> histLine;
        histLine.reserve(1024);
        while (cursorLine < oldCursorLine && m_history->lineCount()) {
            const int histPos = m_history->lineCount() - 1;
            const int histLineLen = m_history->lineLength(histPos);
            const quint8 lineProperty = m_history->lineFlags(histPos);
            histLine.resize(histLineLen);
            m_history->lineCells(histPos, 0, histLineLen, histLine.data());
            m_lines.insert(0, histLine);
            m_lineFlags.insert(m_lineFlags.begin(), lineProperty);
            m_history->dropLastLine();
            cursorLine++;
        }
    }

    m_lineFlags.resize(newLines + 1);
    if (m_lineFlags.size() > m_lines.size())
        std::fill(m_lineFlags.begin() + m_lines.size(), m_lineFlags.end(), LineDefault);
    m_lines.resize(newLines + 1);

    m_lineCount = newLines;
    m_columnCount = newColumns;
    m_cursorColumn = qMin(m_cursorColumn, m_columnCount - 1);
    cursorLine = qBound(0, cursorLine, m_lineCount - 1);
    setCursorLine(cursorLine);

    setDefaultMargins();
    initTabStops();
    clearSelection();
}

bool DTerminalScreen::consumeResizeFlag()
{
    if (m_resizePending) {
        m_resizePending = false;
        return true;
    }
    return false;
}

// ---- 区域搬移与清空 ----

void DTerminalScreen::clearRange(int from, int to, char fillChar, bool resetLineFlags)
{
    const int screenTop = cellIndex(0, m_history->lineCount());

    // 与待搬移区域重叠的选区直接清除
    if ((m_selectionBottomRight > (from + screenTop)) && (m_selectionTopLeft < (to + screenTop)))
        clearSelection();

    const int topLine = from / m_columnCount;
    const int bottomLine = to / m_columnCount;

    const DTerminalChar clearChar(fillChar, StylePlain, m_currentFore, m_currentBack);

    // 清空字符与默认字符一致时可直接收缩行
    const bool isDefaultCh = (clearChar == DTerminalChar());

    for (int y = topLine; y <= bottomLine; ++y) {
        if (y >= m_lineFlags.size())
            break;
        m_lineFlags[y] = LineDefault;

        const int endCol = (y == bottomLine) ? to % m_columnCount : m_columnCount - 1;
        const int startCol = (y == topLine) ? from % m_columnCount : 0;

        QVector<DTerminalChar> &line = m_lines[y];

        if (isDefaultCh && endCol == m_columnCount - 1) {
            line.resize(startCol);
        } else {
            if (line.size() < endCol + 1)
                line.resize(endCol + 1);

            DTerminalChar *data = line.data();
            for (int i = startCol; i <= endCol; ++i)
                data[i] = clearChar;
        }

        if (resetLineFlags && startCol == 0 && endCol == m_columnCount - 1)
            m_lineFlags[y] &= ~(LineDoubleWidth | LineDoubleHeightTop | LineDoubleHeightBottom);
    }
}

void DTerminalScreen::moveRange(int dest, int sourceBegin, int sourceEnd)
{
    Q_ASSERT(sourceBegin <= sourceEnd);

    const int lines = (sourceEnd - sourceBegin) / m_columnCount;

    // 源与目标可能重叠，按方向选择拷贝顺序
    if (dest < sourceBegin) {
        for (int i = 0; i <= lines; ++i) {
            m_lines[(dest / m_columnCount) + i] = m_lines[(sourceBegin / m_columnCount) + i];
            m_lineFlags[(dest / m_columnCount) + i] = m_lineFlags[(sourceBegin / m_columnCount) + i];
        }
    } else {
        for (int i = lines; i >= 0; --i) {
            m_lines[(dest / m_columnCount) + i] = m_lines[(sourceBegin / m_columnCount) + i];
            m_lineFlags[(dest / m_columnCount) + i] = m_lineFlags[(sourceBegin / m_columnCount) + i];
        }
    }

    if (m_lastPos != -1) {
        const int diff = dest - sourceBegin;
        m_lastPos += diff;
        if (m_lastPos < 0 || m_lastPos >= (lines * m_columnCount))
            m_lastPos = -1;
    }

    // 选区随滚动调整
    if (m_selectionAnchor != -1) {
        const bool beginIsTL = (m_selectionAnchor == m_selectionTopLeft);
        const int diff = dest - sourceBegin;
        const int screenTop = cellIndex(0, m_history->lineCount());
        const int srcBegin = sourceBegin + screenTop;
        const int srcEnd = sourceEnd + screenTop;
        const int destBegin = srcBegin + diff;
        const int destEnd = srcEnd + diff;

        if (m_selectionTopLeft >= srcBegin && m_selectionTopLeft <= srcEnd)
            m_selectionTopLeft += diff;
        else if (m_selectionTopLeft >= destBegin && m_selectionTopLeft <= destEnd)
            m_selectionBottomRight = -1;  // 清除选区

        if (m_selectionBottomRight >= srcBegin && m_selectionBottomRight <= srcEnd)
            m_selectionBottomRight += diff;
        else if (m_selectionBottomRight >= destBegin && m_selectionBottomRight <= destEnd)
            m_selectionBottomRight = -1;

        if (m_selectionBottomRight < 0) {
            clearSelection();
        } else {
            if (m_selectionTopLeft < 0)
                m_selectionTopLeft = 0;
        }

        if (beginIsTL)
            m_selectionAnchor = m_selectionTopLeft;
        else
            m_selectionAnchor = m_selectionBottomRight;
    }
}
