// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DTERMINALSCREEN_H
#define DTERMINALSCREEN_H

#include "dterminalhistory.h"
#include "dterminalutil.h"
#include "dtextfilter.h"

#include <QBitArray>
#include <QRect>

#include <array>
#include <functional>

class DTerminalView;

// 屏幕模式（下标即取值，与历史实现一致）
enum DTerminalScreenMode {
    ModeOrigin = 0,      // 光标相对滚动区定位
    ModeWrap,            // 列尾自动换行
    ModeInsert,          // 插入模式
    ModeInverseVideo,    // 反显
    ModeShowCursor,      // 光标可见
    ModeNewLine,         // 换行即回车
    ModeAlternate,       // 备用屏（光标状态存于保存态）
    ModeCount
};

/*******************************************************************************
 1. @类名:    DTerminalScreen
 2. @作者:    ut001213 liuzhangjian
 3. @日期:    2026-11-13
 4. @说明:    终端屏幕缓冲（自研实现）。持有当前屏幕图像（每行
            变长字符单元 + 行属性）、滚动历史、光标与外观状态、
            选区模型，支持 VT 屏幕指令与尺寸变更回绕。
*******************************************************************************/
class DTerminalScreen
{
public:
    /** 文本提取选项 */
    enum TextOption {
        TextPlain = 0x0,          // 纯文本
        TextHtml = 0x1,           // HTML 文本
        TextKeepBreaks = 0x2,     // 行尾保留换行符
        TextTrimLeading = 0x4,    // 去除行首空白
        TextTrimTrailing = 0x8    // 去除行尾空白
    };
    Q_DECLARE_FLAGS(TextOptions, TextOption)

    DTerminalScreen(int lines, int columns);
    ~DTerminalScreen();

    // ---- 光标移动 ----
    void cursorUp(int n);
    void cursorDown(int n);
    void cursorLeft(int n);
    void cursorRight(int n);
    void setCursorRow(int row);
    void setCursorColumn(int column);
    void setCursorPosition(int row, int column);

    // ---- 滚动区 ----
    void setMargins(int topLine, int bottomLine);
    int topMargin() const { return m_topMargin; }
    int bottomMargin() const { return m_bottomMargin; }
    void setDefaultMargins();

    void newLine();
    void nextLine();
    void index();
    void reverseIndex();
    void scrollUp(int n);
    void scrollDown(int n);
    void toStartOfLine();
    void backspace();
    void tab(int n = 1);
    void backtab(int n);

    // ---- 编辑 ----
    void eraseChars(int n);
    void deleteChars(int n);
    void insertChars(int n);
    void repeatChars(int count);
    void deleteLines(int n);
    void insertLines(int n);
    void clearTabStops();
    void changeTabStop(bool set);

    // ---- 模式 ----
    void resetMode(DTerminalScreenMode mode);
    void setMode(DTerminalScreenMode mode);
    void saveMode(DTerminalScreenMode mode);
    void restoreMode(DTerminalScreenMode mode);
    bool getMode(DTerminalScreenMode mode) const;

    // ---- 光标保存/恢复 ----
    void saveCursor();
    void restoreCursor();

    // ---- 清屏 ----
    void clearEntireScreen();
    void clearToEndOfScreen();
    void clearToBeginOfScreen();
    void clearEntireLine();
    void clearToEndOfLine();
    void clearToBeginOfLine();
    void helpAlign();

    // ---- 外观 ----
    void setRendition(quint16 rendition);
    void resetRendition(quint16 rendition);
    void setForeColor(int space, int color);
    void setBackColor(int space, int color);
    void setDefaultRendition();

    int cursorColumn() const { return m_cursorColumn; }
    int cursorRow() const { return m_cursorRow; }

    void clear();
    void home();
    void reset(bool clearScreen = true);

    /** 在当前光标处显示一个字符（含自动换行与宽字符占位） */
    void displayCharacter(uint c);

    // ---- 尺寸 ----
    void resizeImage(int newLines, int newColumns);

    // ---- 图像快照 ----
    void getImage(DTerminalChar *dest, int size, int startLine, int endLine) const;
    QVector<quint8> getLineFlags(int startLine, int endLine) const;
    int lineCount() const { return m_lineCount; }
    int columnCount() const { return m_columnCount; }
    int historyLineCount() const;

    // ---- 历史 ----
    void setHistoryStorage(DTerminalHistory::StorageKind kind, int maxLines, bool copyContent = true);
    /** 以当前存储方式重建历史（内容清空） */
    void clearHistoryContent();
    DTerminalHistory *history() { return m_history; }
    bool hasHistory() const;

    // ---- 选区 ----
    void setSelectionStart(int column, int line, bool blockSelectionMode);
    void setSelectionEnd(int column, int line);
    /** 屏幕全选（deepin 自有功能移植） */
    void setSelectionAll();
    void getSelectionStart(int &column, int &line) const;
    void getSelectionEnd(int &column, int &line) const;
    void clearSelection();
    bool isSelected(int column, int line) const;
    QString selectedText(TextOptions options = TextPlain) const;
    QString text(int startIndex, int endIndex, TextOptions options = TextPlain) const;
    void checkSelection(int from, int to);

    /**
     * 提取 [startLine, endLine] 的纯文本（软换行行不加分隔，其余行尾加换行；
     * 末行数据短于整列宽时补换行并记录空行位置——与流式输出的语义一致）。
     * lineStarts 非空时记录每段文本的起始位置（含可能的末尾空行）。
     */
    QString extractPlainText(int startLine, int endLine, QList<int> *lineStarts = nullptr) const;

    // ---- 行属性 ----
    void setLineFlag(quint8 flag, bool enable);

    // ---- 滚动统计 ----
    int scrolledLines() const { return m_scrolledLines; }
    QRect lastScrolledRegion() const { return m_lastScrolledRegion; }
    void resetScrolledLines() { m_scrolledLines = 0; }
    int droppedLines() const { return m_droppedLines; }
    void resetDroppedLines() { m_droppedLines = 0; }

    // ---- 协作接口 ----
    void setSequenceResolver(const std::function<QString(uint)> &resolver) { m_sequenceResolver = resolver; }
    DUrlExtractor *urlExtractor() { return &m_urlExtractor; }
    void setView(DTerminalView *view) { m_view = view; }
    DTerminalView *view() { return m_view; }

    /** 会话前台进程名提供者（resize 时 zsh 光标修正用，M4 注入） */
    void setForegroundNameProvider(const std::function<QString(int)> &provider) { m_foregroundNameProvider = provider; }
    void setSessionId(int sessionId) { m_sessionId = sessionId; }

    // ---- deepin 扩展：滚动毛刺修复 ----
    int oldTotalLines() { return m_oldTotalLines; }
    bool consumeResizeFlag();
    void setReflowEnabled(bool enable) { m_reflowEnabled = enable; }

    /** 默认字符单元 */
    static DTerminalChar defaultChar();

private:
    DTerminalScreen(const DTerminalScreen &) = delete;
    DTerminalScreen &operator=(const DTerminalScreen &) = delete;

    // 行内定位
    int cellIndex(int line, int column) const { return line * m_columnCount + column; }

    void clearRange(int from, int to, char fillChar, bool resetLineFlags = true);
    void moveRange(int dest, int sourceBegin, int sourceEnd);
    void scrollUpFrom(int from, int n);
    void scrollDownFrom(int from, int n);
    void pushHistoryLine();
    void pushFirstHistoryLine();
    void initTabStops();
    void updateEffectiveStyle();
    void swapColors(DTerminalChar &cell) const;
    bool isSelectionValid() const;
    void writeRange(QString &out, int startIndex, int endIndex, TextOptions options) const;
    void copyFromScreen(DTerminalChar *dest, int startLine, int count) const;
    void copyFromHistory(DTerminalChar *dest, int startLine, int count) const;
    static DTerminalChar *characterBuffer(int size);
    int copyLineToText(QString &out, int line, int start, int count, bool appendNewLine, TextOptions options) const;
    int cursorLine() const;
    void setCursorLine(int newLine);
    int lineLength(int line) const;
    int screenLineColumns(int line) const;

    // ---- 屏幕图像 ----
    int m_lineCount;
    int m_columnCount;

    QVector<QVector<DTerminalChar>> m_lines;   // [lineCount+1] 每行变长
    QVector<quint8> m_lineFlags;               // [lineCount+1]
    int m_scrolledLines = 0;
    QRect m_lastScrolledRegion;
    int m_droppedLines = 0;
    int m_oldTotalLines = 0;
    bool m_resizePending = false;
    bool m_reflowEnabled = true;

    // ---- 历史 ----
    DTerminalHistory *m_history = nullptr;
    DTerminalHistory::StorageKind m_historyKind = DTerminalHistory::StorageKind::None;
    int m_historyMaxLines = -1;

    // ---- 光标与外观 ----
    int m_cursorColumn = 0;
    int m_cursorRow = 0;
    DTerminalCellColor m_currentFore{ColorSpaceDefault, kDefaultForeIndex};
    DTerminalCellColor m_currentBack{ColorSpaceDefault, kDefaultBackIndex};
    quint16 m_currentStyle = StylePlain;
    DTerminalCellColor m_effectiveFore{ColorSpaceDefault, kDefaultForeIndex};
    DTerminalCellColor m_effectiveBack{ColorSpaceDefault, kDefaultBackIndex};
    quint16 m_effectiveStyle = StylePlain;

    // ---- 滚动区 ----
    int m_topMargin = 0;
    int m_bottomMargin = 0;

    // ---- 模式 ----
    std::array<bool, ModeCount> m_modes{};
    std::array<bool, ModeCount> m_savedModes{};

    QBitArray m_tabStops;

    // ---- 选区（图像内偏移）----
    int m_selectionAnchor = -1;
    int m_selectionTopLeft = -1;
    int m_selectionBottomRight = -1;
    bool m_blockSelection = false;

    // ---- 保存态 ----
    struct SavedCursorState {
        int cursorColumn = 0;
        int cursorRow = 0;
        quint16 style = StylePlain;
        DTerminalCellColor fore{ColorSpaceDefault, kDefaultForeIndex};
        DTerminalCellColor back{ColorSpaceDefault, kDefaultBackIndex};
    };
    SavedCursorState m_savedState;

    // ---- 其他 ----
    int m_lastPos = -1;
    quint32 m_lastDrawnChar = 0;
    DUrlExtractor m_urlExtractor;
    DTerminalView *m_view = nullptr;
    int m_sessionId = 0;
    std::function<QString(uint)> m_sequenceResolver;
    std::function<QString(int)> m_foregroundNameProvider;

    static const int kMaxScreenArgument = 40960;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(DTerminalScreen::TextOptions)

#endif  // DTERMINALSCREEN_H
