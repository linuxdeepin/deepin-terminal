// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DTERMINALPARSER_H
#define DTERMINALPARSER_H

#include "dterminalscreen.h"
#include "dkeyboardtranslator.h"

#include <QObject>
#include <QSize>
#include <QHash>
#include <QSet>

#include <array>
#include <functional>

class QTextCodec;
class QTextDecoder;
class QTimer;
class QKeyEvent;

// 终端活动状态（与历史实现的 NOTIFY* 取值对齐）
enum DTerminalActivity {
    ActivityNormal = 0,
    ActivityBell = 1,
    ActivityBusy = 2
};

// 解析器级模式（终端态，区别于屏幕态）
enum DTerminalParserMode {
    ParserModeAppCursorKeys = 0,   // DECCKM 应用光标键
    ParserModeAppKeypad,           // 应用小键盘
    ParserModeMouse1000,
    ParserModeMouse1001,
    ParserModeMouse1002,
    ParserModeMouse1003,
    ParserModeMouse1005,
    ParserModeMouse1006,
    ParserModeMouse1007,
    ParserModeMouse1015,
    ParserModeAnsi,                // DECANM
    ParserModeAppScreen,           // 备用屏
    ParserMode132Columns,          // DECCOLM
    ParserModeAllow132Columns,
    ParserModeBracketedPaste,
    ParserModeCount
};

/*******************************************************************************
 1. @类名:    DCharSequenceTable
 2. @说明:    组合字符序列哈希表。屏幕单元格以哈希引用多码点序列，
            此处按哈希存储与查重；哈希耗尽时经收集器清理未用项。
*******************************************************************************/
class DCharSequenceTable
{
public:
    static DCharSequenceTable *instance();

    /** 登记序列并返回哈希（0 表示失败） */
    uint registerSequence(const uint *points, ushort length);
    /** 按哈希取回序列；返回序列首指针并写回长度，未命中返回 nullptr */
    const uint *lookup(uint hash, ushort &length) const;

    /** 已用哈希收集器（M5 由渲染层注入，用于哈希耗尽时清理） */
    void setUsedCharsCollector(const std::function<QSet<uint>()> &collector) { m_usedCollector = collector; }

private:
    DCharSequenceTable();
    ~DCharSequenceTable();

    static uint hashSequence(const uint *points, ushort length);
    bool sequenceMatches(uint hash, const uint *points, ushort length) const;

    QHash<uint, uint *> m_table;
    std::function<QSet<uint>()> m_usedCollector;
};

/*******************************************************************************
 2. @类名:    DTerminalParser
 3. @作者:    ut001213 liuzhangjian
 4. @日期:    2026-11-13
 5. @说明:    终端 VT 解析器（自研实现）。消费 shell 输出字节流，
            解码后经词法分析拆分为指令，驱动主/备两块屏幕；
            同时负责键盘/鼠标事件到转义序列的转换。
*******************************************************************************/
class DTerminalParser : public QObject
{
    Q_OBJECT
public:
    explicit DTerminalParser(QObject *parent = nullptr);
    ~DTerminalParser() override;

    // ---- 屏幕 ----
    DTerminalScreen *currentScreen() const { return m_screen[m_currentScreenIndex]; }
    DTerminalScreen *primaryScreen() const { return m_screen[0]; }
    DTerminalScreen *alternateScreen() const { return m_screen[1]; }
    void setImageSize(int lines, int columns);
    QSize imageSize() const;
    int lineCount() const;
    int columnCount() const;
    void setHistoryStorage(DTerminalHistory::StorageKind kind, int maxLines, bool copyContent);
    void clearHistory();
    void clearEntireScreen();
    void reset();

    // ---- 编解码 ----
    void setCodec(QTextCodec *codec);
    const QTextCodec *codec() const { return m_codec; }
    bool isUtf8() const;

    // ---- 数据通路 ----
    void feedData(const char *buffer, int length, bool isCommandExec = false);

    // ---- 键盘与鼠标 ----
    void handleKeyEvent(QKeyEvent *event);
    void handleMouseEvent(int buttons, int column, int line, int eventType);
    void sendText(const QString &text);
    void focusGained();
    void focusLost();
    void setKeyTranslator(DKeyboardTranslator *translator);
    DKeyboardTranslator *keyTranslator() const { return m_keyTranslator; }
    void setBackspaceSequence(char *key, int length);
    void setDeleteSequence(char *key, int length);
    char eraseChar() const;

    // ---- 状态 ----
    bool programUsesMouse() const { return m_usesMouse; }
    bool programBracketedPasteMode() const { return m_bracketedPaste; }

    // ---- deepin 扩展 ----
    void setSessionId(int sessionId) { m_sessionId = sessionId; }
    /** bash 长提示符光标修正所需的前台进程名提供者（M4 注入） */
    void setForegroundNameProvider(const std::function<QString(int)> &provider) { m_foregroundNameProvider = provider; }
    /** GB18030 2005 标准工作区编码转换器（M4 由编码检测模块注入） */
    void setEncodingConverter(const std::function<QByteArray(const QByteArray &, const QString &, const QString &)> &converter)
    { m_encodingConverter = converter; }

signals:
    void dataToSend(const char *data, int length, const QTextCodec *codec);
    void lockPtyRequest(bool suspend);
    void useUtf8Request(bool useUtf8);
    void activityChanged(int state);
    void zmodemDetected();
    void changeTabTextColorRequest(int color);
    void programUsesMouseChanged(bool usesMouse);
    void alternateScrollingEnabled(bool enable);
    void programBracketedPasteModeChanged(bool enabled);
    void outputChanged();
    void titleChanged(int what, const QString &value);
    void imageSizeChanged(int lines, int columns);
    void imageResizeRequest(const QSize &size);
    void profileChangeCommandReceived(const QString &text);
    void flowControlKeyPressed(bool suspend);
    void primaryScreenInUse(bool use);
    void cursorChanged(int cursorShape, bool blinkingCursorEnabled);
    void osc52ClipboardRequest(char target, const QString &base64Data);

protected:
    void receiveChar(wchar_t c);
    void setParserMode(DTerminalParserMode mode);
    void resetParserMode(DTerminalParserMode mode);
    void saveParserMode(DTerminalParserMode mode);
    void restoreParserMode(DTerminalParserMode mode);
    bool getParserMode(DTerminalParserMode mode) const;

private:
    // ---- 词法分析 ----
    enum TokenKind {
        TokenChar = 0, TokenControl = 1, TokenEscape = 2, TokenEscapeCharset = 3,
        TokenEscapeHash = 4, TokenCsiParam = 5, TokenCsiPlain = 6, TokenCsiPrivate = 7,
        TokenVt52 = 8, TokenCsiGreater = 9, TokenCsiBang = 10, TokenCsiParamSpace = 11
    };
    static constexpr int makeToken(TokenKind kind, int a, int n)
    {
        return ((n & 0xffff) << 16) | ((a & 0xff) << 8) | (static_cast<int>(kind) & 0xff);
    }

    void resetTokenizer();
    void addDigit(int digit);
    void addArgument();
    void addToCurrentToken(wchar_t c);
    void initCharClasses();

    // ---- 指令执行 ----
    void processToken(int token, wchar_t p, int q);
    void processOsc();
    void flushTitleUpdates();

    // ---- 应答 ----
    void reportCursorPosition();
    void reportTerminalType();
    void reportSecondaryAttributes();
    void reportTerminalParms(int p);
    void reportStatus();
    void reportAnswerBack();
    void reportAnsiMode(int mode, int status);
    void reportDecMode(int mode, int status);
    void reportDecodingError();

    // ---- OSC52 ----
    bool isOsc52Start() const;
    char extractOsc52Target() const;
    QString extractOsc52Data() const;
    void startOsc52();
    void handleOsc52End();
    void handleOsc52Chunk();

    // ---- 字符集 ----
    wchar_t applyCharset(wchar_t c);
    void setCharset(int n, int cs);
    void useCharset(int n);
    void setAndUseCharset(int n, int cs);
    void resetCharset(int screenIndex);
    void saveCursorState();
    void restoreCursorState();

    void setMargins(int top, int bottom);
    void setDefaultMargins();
    void resetParserModes();
    void clearScreenAndSetColumns(int columnCount);
    void sendString(const char *string, int length = -1);
    void bufferedUpdate();
    void showBulk();
    void activateScreen(int index);
    void checkScreenInUse();
    static bool detectIconv2005Standard();

    // ---- 词法状态 ----
    wchar_t m_tokenBuffer[256];
    int m_tokenBufferPos = 0;
    int m_argv[16];
    int m_argc = 0;
    int m_prevCC = 0;
    int m_charClass[256];

    // ---- 屏幕 ----
    DTerminalScreen *m_screen[2];
    int m_currentScreenIndex = 0;

    // ---- 字符集状态（每屏一份）----
    struct CharsetState {
        char charset[4] = {'B', 'B', 'B', 'B'};
        int active = 0;
        bool graphic = false;
        bool pound = false;
        bool savedGraphic = false;
        bool savedPound = false;
    };
    CharsetState m_charset[2];

    // ---- 模式 ----
    std::array<bool, ParserModeCount> m_modes{};
    std::array<bool, ParserModeCount> m_savedModes{};

    // ---- 编解码 ----
    QTextCodec *m_codec = nullptr;
    QTextDecoder *m_decoder = nullptr;

    // ---- 键盘 ----
    DKeyboardTranslator *m_keyTranslator = nullptr;

    // ---- OSC 缓冲 ----
    QHash<int, QString> m_pendingTitles;
    QTimer *m_titleTimer = nullptr;

    bool m_reportFocusEvents = false;
    bool m_usesMouse = false;
    bool m_alternateScrolling = true;
    bool m_bracketedPaste = false;
    QTimer *m_bulkTimer1 = nullptr;
    QTimer *m_bulkTimer2 = nullptr;
    int m_sessionId = 0;

    // ---- OSC52 ----
    bool m_osc52InProgress = false;
    QString m_osc52Data;
    char m_osc52Target = 'c';
    bool m_osc52FirstChunk = false;

    std::function<QString(int)> m_foregroundNameProvider;
    std::function<QByteArray(const QByteArray &, const QString &, const QString &)> m_encodingConverter;

    static const int kMaxTokenLength = 256;
    static const int kMaxArguments = 15;
    static const int kMaxArgument = 4096;
    static const int kMaxOsc52Buffer = 64 * 1024 * 1024;
};

#endif  // DTERMINALPARSER_H
