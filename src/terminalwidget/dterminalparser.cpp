// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dterminalparser.h"

#include <QDebug>
#include <QKeyEvent>
#include <QTextCodec>
#include <QTimer>

#include <cstdio>
#include <cstring>
#include <iconv.h>

// 终端解析流水线说明：
//   字节流 → 解码 → 字符分类（词法）→ 指令（token）→ 屏幕操作/信号。
// 词法状态由"已累积的 token 缓冲 + 当前字符"共同推导：
//   普通字符按当前字符集翻译后落屏；ESC 起始序列按后续字符分类
//   逐步收敛为 ESC/CSI/OSC/VT52 指令；OSC 以 BEL 或 ST 终止。
// 支持 xterm 扩展：鼠标协议、焦点上报、括贴模式、DECRQM 查询、
// 24 位真彩/256 色、OSC52 剪贴板（分块，deepin 自有扩展）。

namespace {

constexpr wchar_t kEscape = 27;
constexpr wchar_t kDelete = 127;
constexpr wchar_t kBell = 7;

// VT100 专用图形字符集（0x5F-0x7E 的标准映射数据）
constexpr ushort kVt100Graphics[32] = {
    0x0020, 0x25C6, 0x2592, 0x2409, 0x240C, 0x240D, 0x240A, 0x00B0,
    0x00B1, 0x2424, 0x240B, 0x2518, 0x2510, 0x250C, 0x2514, 0x253C,
    0x23BA, 0x23BB, 0x2500, 0x23BC, 0x23BD, 0x251C, 0x2524, 0x2534,
    0x252C, 0x2502, 0x2264, 0x2265, 0x03C0, 0x2260, 0x00A3, 0x00B7
};

// 字符类别位
constexpr int kClassControl = 1;
constexpr int kClassPrintable = 2;
constexpr int kClassCsiParam = 4;   // CSI 参数终结符
constexpr int kClassDigit = 8;
constexpr int kClassCharsetSel = 16;
constexpr int kClassEscapeGroup = 32;
constexpr int kClassResizeEnd = 64; // \e[8;<row>;<col>t 的终止符

// 控制字符转义名（^X → 0x18）
constexpr wchar_t controlKey(wchar_t c) { return c - '@'; }

}  // namespace

// ---- 组合字符序列表 ----

DCharSequenceTable::DCharSequenceTable() = default;

DCharSequenceTable::~DCharSequenceTable()
{
    QHashIterator<uint, uint *> iter(m_table);
    while (iter.hasNext()) {
        iter.next();
        delete[] iter.value();
    }
}

DCharSequenceTable *DCharSequenceTable::instance()
{
    static DCharSequenceTable table;
    return &table;
}

uint DCharSequenceTable::hashSequence(const uint *points, ushort length)
{
    uint hash = 0;
    for (ushort i = 0; i < length; ++i)
        hash = 31 * hash + points[i];
    return hash;
}

bool DCharSequenceTable::sequenceMatches(uint hash, const uint *points, ushort length) const
{
    const uint *entry = m_table.value(hash, nullptr);
    if (entry == nullptr || entry[0] != length)
        return false;
    for (ushort i = 0; i < length; ++i) {
        if (entry[i + 1] != points[i])
            return false;
    }
    return true;
}

uint DCharSequenceTable::registerSequence(const uint *points, ushort length)
{
    uint hash = hashSequence(points, length);
    const uint initialHash = hash;
    bool triedCleaning = false;

    // 线性探测；0 号哈希保留（单元格码点 0 表示宽字符延续格）
    while (m_table.contains(hash) && hash != 0) {
        if (sequenceMatches(hash, points, length))
            return hash;
        hash++;

        if (hash == initialHash) {
            if (!triedCleaning) {
                triedCleaning = true;
                // 哈希耗尽：收集所有屏幕仍引用的哈希，清理其余项
                QSet<uint> usedChars;
                if (m_usedCollector)
                    usedChars = m_usedCollector();

                QMutableHashIterator<uint, uint *> it(m_table);
                while (it.hasNext()) {
                    it.next();
                    if (!usedChars.contains(it.key())) {
                        delete[] it.value();
                        it.remove();
                    }
                }
            } else {
                qDebug() << "Using all the extended char hashes, going to miss this extended character";
                return 0;
            }
        }
    }

    auto *buffer = new (std::nothrow) uint[length + 1];
    if (buffer == nullptr) {
        qCritical() << "DTerminalParser: failed to allocate extended character buffer"
                    << (length + 1) << "uints";
        return 0;
    }
    buffer[0] = length;
    for (ushort i = 0; i < length; ++i)
        buffer[i + 1] = points[i];

    m_table.insert(hash, buffer);
    return hash;
}

const uint *DCharSequenceTable::lookup(uint hash, ushort &length) const
{
    const uint *buffer = m_table.value(hash, nullptr);
    if (buffer != nullptr) {
        length = ushort(buffer[0]);
        return buffer + 1;
    }
    length = 0;
    return nullptr;
}

// ---- 解析器 ----

DTerminalParser::DTerminalParser(QObject *parent)
    : QObject(parent)
    , m_titleTimer(new QTimer(this))
    , m_bulkTimer1(new QTimer(this))
    , m_bulkTimer2(new QTimer(this))
{
    m_screen[0] = new DTerminalScreen(40, 80);
    m_screen[1] = new DTerminalScreen(40, 80);
    m_screen[0]->setView(nullptr);
    m_screen[1]->setView(nullptr);

    m_titleTimer->setSingleShot(true);
    connect(m_titleTimer, &QTimer::timeout, this, &DTerminalParser::flushTitleUpdates);

    m_bulkTimer1->setSingleShot(true);
    m_bulkTimer2->setSingleShot(true);
    connect(m_bulkTimer1, &QTimer::timeout, this, &DTerminalParser::showBulk);
    connect(m_bulkTimer2, &QTimer::timeout, this, &DTerminalParser::showBulk);

    // 光标形状变化同步进标题参数（供上层标签/配置感知）
    connect(this, &DTerminalParser::cursorChanged, this, [this](int cursorShape, bool blink) {
        emit titleChanged(50, QStringLiteral("CursorShape=%1;BlinkingCursorEnabled=%2")
                          .arg(cursorShape).arg(blink));
    });

    initCharClasses();
    resetTokenizer();
    reset();
}

DTerminalParser::~DTerminalParser()
{
    delete m_screen[0];
    delete m_screen[1];
    delete m_decoder;
}

// ---- 词法 ----

void DTerminalParser::initCharClasses()
{
    for (int &entry : m_charClass)
        entry = 0;
    for (int i = 0; i < 32; ++i)
        m_charClass[i] |= kClassControl;
    for (int i = 32; i < 256; ++i)
        m_charClass[i] |= kClassPrintable;

    for (const char *s = "@ABCDGHILMPSTXZbcdfry"; *s; ++s)
        m_charClass[static_cast<int>(*s)] |= kClassCsiParam;
    // resize = \e[8;<row>;<col>t
    for (const char *s = "t"; *s; ++s)
        m_charClass[static_cast<int>(*s)] |= kClassResizeEnd;
    for (const char *s = "0123456789"; *s; ++s)
        m_charClass[static_cast<int>(*s)] |= kClassDigit;
    for (const char *s = "()+*%"; *s; ++s)
        m_charClass[static_cast<int>(*s)] |= kClassCharsetSel;
    for (const char *s = "()+*#[]%"; *s; ++s)
        m_charClass[static_cast<int>(*s)] |= kClassEscapeGroup;

    resetTokenizer();
}

void DTerminalParser::resetTokenizer()
{
    m_tokenBufferPos = 0;
    m_argc = 0;
    m_argv[0] = 0;
    m_argv[1] = 0;
    m_prevCC = 0;
}

void DTerminalParser::addDigit(int digit)
{
    if (m_argv[m_argc] < kMaxArgument)
        m_argv[m_argc] = 10 * m_argv[m_argc] + digit;
}

void DTerminalParser::addArgument()
{
    m_argc = qMin(m_argc + 1, kMaxArguments - 1);
    m_argv[m_argc] = 0;
}

void DTerminalParser::addToCurrentToken(wchar_t c)
{
    m_tokenBuffer[m_tokenBufferPos] = c;
    m_tokenBufferPos = qMin(m_tokenBufferPos + 1, kMaxTokenLength - 1);

    // OSC52：缓冲接近上限时切块
    if (m_osc52InProgress && m_tokenBufferPos >= kMaxTokenLength - 100)
        handleOsc52Chunk();
}

// ---- 字符接收（词法核心） ----

void DTerminalParser::receiveChar(wchar_t c)
{
    if (c == kDelete)
        return;  // VT100：DEL 忽略

    // OSC52 后续分块直接累积
    if (m_osc52InProgress && c != kBell) {
        addToCurrentToken(c);
        return;
    }
    // OSC52 以 BEL 收尾
    if (c == kBell && m_osc52InProgress) {
        handleOsc52End();
        resetTokenizer();
        return;
    }

    const bool inOscText = (m_tokenBufferPos >= 2 && m_tokenBuffer[1] == L']');
    const bool stringTerminator = inOscText
            && (c == kBell || (m_prevCC == kEscape && c == 0x5C));

    const bool isControl = (c < 256 && (m_charClass[c] & kClassControl) == kClassControl
                            && !stringTerminator);
    if (isControl) {
        // OSC 文本段中的控制字符按 xterm 语义忽略
        if (inOscText) {
            m_prevCC = c;
            return;
        }

        // CAN/SUB/ESC 打断当前序列
        if (c == controlKey(L'X') || c == controlKey(L'Z') || c == kEscape)
            resetTokenizer();
        if (c != kEscape) {
            processToken(makeToken(TokenControl, c + '@', 0), 0, 0);
            return;
        }
    }

    // 累积当前字符并推进状态
    addToCurrentToken(c);

    wchar_t *s = m_tokenBuffer;
    const int p = m_tokenBufferPos;

    if (getParserMode(ParserModeAnsi)) {
        // 状态推导（按 token 长度与已见字符判定序列类型）
        const bool atEscapeStart = (p == 1 && s[0] == kEscape);
        const bool atEsc128 = (p == 1 && s[0] == kEscape + 128);
        const bool inEscapeGroup = (p == 2 && s[1] < 256 && (m_charClass[s[1]] & kClassEscapeGroup));
        const bool plainChar = (p == 1 && c >= 32);
        const bool twoCharEscape = (p == 2 && s[0] == kEscape);
        const bool charsetSelect = (p == 3 && s[1] < 256 && (m_charClass[s[1]] & kClassCharsetSel));
        const bool hashEscape = (p == 3 && s[1] == '#');
        const bool plainCsiParam = (p >= 3 && s[2] != '?' && s[2] != '!' && s[2] != '>'
                                    && s[2] != '<' && s[2] != '=' && c < 256
                                    && (m_charClass[c] & kClassCsiParam));
        const bool privateCsi = (p >= 3 && s[2] == '?');
        const bool bangCsi = (p >= 3 && s[2] == '!');
        const bool greaterCsi = (p >= 3 && s[2] == '>');
        const bool spaceCsi = (p == 4 && s[3] == ' ');
        const bool isDigit = (p >= 3 && c < 256 && (m_charClass[c] & kClassDigit));
        const bool isSeparator = (c == ';' || c == ':');
        const bool resizeEnd = (p >= 3 && s[2] != '?' && s[2] != '!' && s[2] != '>'
                                && s[2] != '<' && s[2] != '=' && c < 256
                                && (m_charClass[c] & kClassResizeEnd));

        if (atEscapeStart)
            return;
        if (atEsc128) {
            s[0] = kEscape;
            receiveChar(L'[');
            return;
        }
        if (inEscapeGroup)
            return;

        // OSC 统一处理
        if (inOscText && !m_osc52InProgress) {
            if (isOsc52Start()) {
                startOsc52();
                // 继续累积，不返回
            } else if (c == kBell || (m_prevCC == kEscape && c == 0x5C)) {
                processOsc();
                resetTokenizer();
                return;
            } else {
                m_prevCC = c;
                return;
            }
        }

        if (p == 3 && s[2] == '?')
            return;
        if (p == 3 && s[2] == '>')
            return;
        if (p == 3 && s[2] == '!')
            return;
        if (p == 3 && s[2] == '<')
            return;
        if (p == 3 && s[2] == '=')
            return;

        if (plainChar) {
            processToken(makeToken(TokenChar, 0, 0), applyCharset(c), 0);
            resetTokenizer();
            return;
        }
        if (twoCharEscape) {
            processToken(makeToken(TokenEscape, s[1], 0), 0, 0);
            resetTokenizer();
            return;
        }
        if (charsetSelect) {
            processToken(makeToken(TokenEscapeCharset, s[1], s[2]), 0, 0);
            resetTokenizer();
            return;
        }
        if (hashEscape) {
            processToken(makeToken(TokenEscapeHash, s[2], 0), 0, 0);
            resetTokenizer();
            return;
        }
        if (plainCsiParam) {
            processToken(makeToken(TokenCsiPlain, c, 0), m_argv[0], m_argv[1]);
            resetTokenizer();
            return;
        }
        if (spaceCsi)
            return;
        // DECRQM：CSI Pd $ p 的 '$' 中间字节吸收后等待终结符
        if (p >= 3 && c == '$')
            return;
        if (p == 5 && s[4] == 'q' && s[3] == ' ') {
            processToken(makeToken(TokenCsiParamSpace, c, m_argv[0]), m_argv[0], 0);
            resetTokenizer();
            return;
        }
        // resize = \e[8;<row>;<col>t
        if (resizeEnd) {
            processToken(makeToken(TokenCsiParam, c, m_argv[0]), m_argv[1], m_argv[2]);
            resetTokenizer();
            return;
        }

        if (bangCsi) {
            processToken(makeToken(TokenCsiBang, c, 0), 0, 0);
            resetTokenizer();
            return;
        }
        if (isDigit) {
            addDigit(c - '0');
            return;
        }
        if (isSeparator) {
            addArgument();
            return;
        }
        for (int i = 0; i <= m_argc; ++i) {
            if (privateCsi) {
                processToken(makeToken(TokenCsiPrivate, c, m_argv[i]), 0, 0);
            } else if (greaterCsi) {
                processToken(makeToken(TokenCsiGreater, c, 0), 0, 0);  // ESC]>0c / ESC]>c
            } else if (p >= 3 && (s[2] == '<' || s[2] == '=')) {
                ;  // 静默忽略 CSI < 与 CSI =（如 Kitty 键盘弹窗协议）
            } else if (c == 'm' && m_argc - i >= 4 && (m_argv[i] == 38 || m_argv[i] == 48)
                       && m_argv[i + 1] == 2) {
                // 24 位真彩：38/48;2;<r>;<g>;<b>
                i += 2;
                processToken(makeToken(TokenCsiParam, c, m_argv[i - 2]), 4 /*RGB*/,
                             (m_argv[i] << 16) | (m_argv[i + 1] << 8) | m_argv[i + 2]);
                i += 2;
            } else if (c == 'm' && m_argc - i >= 2 && (m_argv[i] == 38 || m_argv[i] == 48)
                       && m_argv[i + 1] == 5) {
                // 256 色：38/48;5;<index>
                i += 2;
                processToken(makeToken(TokenCsiParam, c, m_argv[i - 2]), 3 /*256*/,
                             m_argv[i]);
            } else {
                processToken(makeToken(TokenCsiParam, c, m_argv[i]), 0, 0);
            }
        }
        resetTokenizer();
    } else {
        // VT52 模式
        const bool vt52Start = (p == 1 && s[0] == kEscape);
        const bool vt52Char = (p == 1 && s[0] < 256 && (m_charClass[s[0]] & kClassPrintable));
        const bool vt52Y1 = (p == 2 && s[1] == 'Y');
        const bool vt52Y2 = (p == 3 && s[1] == 'Y');

        if (vt52Start)
            return;
        if (vt52Char) {
            processToken(makeToken(TokenChar, 0, 0), s[0], 0);
            resetTokenizer();
            return;
        }
        if (vt52Y1)
            return;
        if (vt52Y2)
            return;
        if (p < 4) {
            processToken(makeToken(TokenVt52, s[1], 0), 0, 0);
            resetTokenizer();
            return;
        }
        processToken(makeToken(TokenVt52, s[1], 0), s[2], s[3]);
        resetTokenizer();
        return;
    }
}

// ---- OSC 处理 ----

void DTerminalParser::processOsc()
{
    // 解析 OSC 属性号：ESC ] N ; value BEL/ST
    int attribute = 0;
    int i;
    for (i = 2; i < m_tokenBufferPos
             && m_tokenBuffer[i] >= '0' && m_tokenBuffer[i] <= '9'; ++i) {
        attribute = 10 * attribute + (m_tokenBuffer[i] - '0');
    }

    if (m_tokenBuffer[i] != ';') {
        reportDecodingError();
        return;
    }

    // 值部分：跳过 ';' 与结尾定界符（BEL 已入缓冲；ST 的 ESC 不入缓冲）
    const QString newValue = QString::fromWCharArray(m_tokenBuffer + i + 1, m_tokenBufferPos - i - 2);

    m_pendingTitles[attribute] = newValue;
    m_titleTimer->start(20);
}

void DTerminalParser::flushTitleUpdates()
{
    QHashIterator<int, QString> iter(m_pendingTitles);
    while (iter.hasNext()) {
        iter.next();
        emit titleChanged(iter.key(), iter.value());
    }
    m_pendingTitles.clear();
}

// ---- OSC52 ----

bool DTerminalParser::isOsc52Start() const
{
    // ESC ] 5 2 ;
    return m_tokenBufferPos >= 5
            && m_tokenBuffer[2] == L'5'
            && m_tokenBuffer[3] == L'2'
            && m_tokenBuffer[4] == L';';
}

char DTerminalParser::extractOsc52Target() const
{
    // ESC ] 5 2 ; target ; base64...
    if (m_tokenBufferPos > 5)
        return static_cast<char>(m_tokenBuffer[5]);
    return 'c';  // 缺省 CLIPBOARD
}

QString DTerminalParser::extractOsc52Data() const
{
    // 首块数据从第 7 位起（跳过 ESC ] 5 2 ; t ;），后续块从 0 起
    const int dataStart = m_osc52FirstChunk ? 7 : 0;
    if (m_tokenBufferPos > dataStart)
        return QString::fromWCharArray(m_tokenBuffer + dataStart, m_tokenBufferPos - dataStart);
    return QString();
}

void DTerminalParser::startOsc52()
{
    m_osc52InProgress = true;
    m_osc52Target = extractOsc52Target();
    m_osc52Data.clear();
    m_osc52FirstChunk = true;

    qInfo() << "OSC52: Start, target=" << m_osc52Target;
}

void DTerminalParser::handleOsc52End()
{
    QString base64Data;
    if (m_osc52FirstChunk) {
        // 单块 OSC52：跳过头部 7 字符（BEL 未入缓冲）
        if (m_tokenBufferPos > 7)
            base64Data = QString::fromWCharArray(m_tokenBuffer + 7, m_tokenBufferPos - 7);
    } else {
        // 末块只含 base64 数据
        if (m_tokenBufferPos > 0)
            base64Data = QString::fromWCharArray(m_tokenBuffer, m_tokenBufferPos);
    }

    m_osc52Data += base64Data;

    if (m_osc52Data.size() >= kMaxOsc52Buffer) {
        qWarning() << "OSC52: Buffer overflow (>=" << kMaxOsc52Buffer << "bytes), rejecting";
        m_osc52Data.clear();
        m_osc52InProgress = false;
        m_osc52FirstChunk = false;
        return;
    }

    emit osc52ClipboardRequest(m_osc52Target, m_osc52Data);

    qInfo() << "OSC52: Complete, sent" << m_osc52Data.size() << "bytes";

    m_osc52Data.clear();
    m_osc52InProgress = false;
    m_osc52FirstChunk = false;
}

void DTerminalParser::handleOsc52Chunk()
{
    if (m_osc52Data.size() >= kMaxOsc52Buffer) {
        qWarning() << "OSC52: Buffer overflow (>=" << kMaxOsc52Buffer << "bytes), rejecting";
        m_osc52Data.clear();
        m_osc52InProgress = false;
        m_osc52FirstChunk = false;
        return;
    }

    m_osc52Data += extractOsc52Data();
    m_osc52FirstChunk = false;

    // 清空 token 缓冲继续接收
    resetTokenizer();
}

// ---- 应答 ----

void DTerminalParser::sendString(const char *string, int length)
{
    if (length >= 0)
        emit dataToSend(string, length, m_codec);
    else
        emit dataToSend(string, static_cast<int>(std::strlen(string)), m_codec);
}

void DTerminalParser::reportCursorPosition()
{
    const int size = 20;
    char tmp[size];
    const int r = std::snprintf(tmp, size, "\033[%d;%dR",
                                currentScreen()->cursorRow() + 1,
                                currentScreen()->cursorColumn() + 1);
    if (size <= r)
        qWarning("DTerminalParser::reportCursorPosition: Buffer too small\n");
    sendString(tmp);
}

void DTerminalParser::reportTerminalType()
{
    if (getParserMode(ParserModeAnsi))
        sendString("\033[?1;2c");  // VT100
    else
        sendString("\033/Z");      // VT52
}

void DTerminalParser::reportSecondaryAttributes()
{
    if (getParserMode(ParserModeAnsi))
        sendString("\033[>0;115;0c");
    else
        sendString("\033/Z");
}

void DTerminalParser::reportTerminalParms(int p)
{
    const int size = 100;
    char tmp[size];
    const int r = std::snprintf(tmp, size, "\033[%d;1;1;112;112;1;0x", p);
    if (size <= r)
        qWarning("DTerminalParser::reportTerminalParms: Buffer too small\n");
    sendString(tmp);
}

void DTerminalParser::reportStatus()
{
    sendString("\033[0n");
}

void DTerminalParser::reportAnsiMode(int mode, int status)
{
    const size_t size = 32;
    char tmp[size];
    const size_t r = std::snprintf(tmp, size, "\033[%d;%d$y", mode, status);
    if (r >= size) {
        qWarning("DTerminalParser::reportAnsiMode: Buffer too small\n");
        return;
    }
    sendString(tmp);
}

void DTerminalParser::reportDecMode(int mode, int status)
{
    const size_t size = 32;
    char tmp[size];
    const size_t r = std::snprintf(tmp, size, "\033[?%d;%d$y", mode, status);
    if (r >= size) {
        qWarning("DTerminalParser::reportDecMode: Buffer too small\n");
        return;
    }
    sendString(tmp);
}

void DTerminalParser::reportAnswerBack()
{
    sendString("");
}

void DTerminalParser::reportDecodingError()
{
    if (m_tokenBufferPos == 0 || (m_tokenBufferPos == 1 && (m_tokenBuffer[0] & 0xff) >= 32))
        return;
    qDebug() << "Undecodable sequence:" << QString::fromWCharArray(m_tokenBuffer, m_tokenBufferPos);
}

// ---- 鼠标 ----

void DTerminalParser::handleMouseEvent(int buttons, int column, int line, int eventType)
{
    if (column < 1 || line < 1)
        return;

    int cb = buttons;
    // 非 1006 模式按下释放编码在按键值中
    if (eventType == 2 && !getParserMode(ParserModeMouse1006))
        cb = 3;

    // 滚轮按键 4/5 编码为 0x5c + button
    if (cb >= 4)
        cb += 0x3c;

    // 拖动事件在 1002/1003 模式下加 32
    if ((getParserMode(ParserModeMouse1002) || getParserMode(ParserModeMouse1003)) && eventType == 1)
        cb += 0x20;

    char command[32];
    command[0] = '\0';
    if (getParserMode(ParserModeMouse1006)) {
        std::snprintf(command, sizeof(command), "\033[<%d;%d;%d%c", cb, column, line,
                      eventType == 2 ? 'm' : 'M');
    } else if (getParserMode(ParserModeMouse1015)) {
        std::snprintf(command, sizeof(command), "\033[%d;%d;%dM",
                      static_cast<short>(cb) + 0x20, column, line);
    } else if (getParserMode(ParserModeMouse1005)) {
        if (column <= 2015 && line <= 2015) {
            // UTF-8 编码坐标（与语言环境无关）
            QChar coords[2];
            coords[0] = static_cast<ushort>(column + 0x20);
            coords[1] = static_cast<ushort>(line + 0x20);
            const QString coordsStr = QString(coords, 2);
            const QByteArray utf8 = coordsStr.toUtf8();
            std::snprintf(command, sizeof(command), "\033[M%c%s", cb + 0x20, utf8.constData());
        }
    } else if (column <= 223 && line <= 223) {
        std::snprintf(command, sizeof(command), "\033[M%c%c%c", cb + 0x20, column + 0x20, line + 0x20);
    }

    sendString(command);
}

// ---- 焦点 ----

void DTerminalParser::focusGained()
{
    if (m_reportFocusEvents)
        sendString("\033[I");
}

void DTerminalParser::focusLost()
{
    if (m_reportFocusEvents)
        sendString("\033[O");
}

// ---- 键盘 ----

void DTerminalParser::sendText(const QString &text)
{
    if (text.isEmpty())
        return;

    QKeyEvent event(QEvent::KeyPress, 0, Qt::NoModifier, text);
    handleKeyEvent(&event);
}

void DTerminalParser::handleKeyEvent(QKeyEvent *event)
{
    const Qt::KeyboardModifiers modifiers = event->modifiers();
    quint16 states = KeyStateNone;

    if (currentScreen()->getMode(ModeNewLine))
        states |= KeyStateNewLine;
    if (getParserMode(ParserModeAnsi))
        states |= KeyStateAnsi;
    if (getParserMode(ParserModeAppCursorKeys))
        states |= KeyStateCursorKeys;
    if (getParserMode(ParserModeAppScreen))
        states |= KeyStateAlternate;
    if (getParserMode(ParserModeAppKeypad) && (modifiers & Qt::KeypadModifier))
        states |= KeyStateAppKeypad;

    // 流控组合键
    if (modifiers & Qt::ControlModifier) {
        switch (event->key()) {
        case Qt::Key_S:
            emit flowControlKeyPressed(true);
            break;
        case Qt::Key_Q:
        case Qt::Key_C:  // 取消流控
            emit flowControlKeyPressed(false);
            break;
        default:
            break;
        }
    }

    if (m_keyTranslator != nullptr) {
        const DKeyEntry entry = m_keyTranslator->findEntry(event->key(), modifiers, states);

        QByteArray textToSend;

        // Alt/Meta 特例：无专门绑定时前置 ESC/设置前缀
        const bool wantsAltModifier = entry.modifiers() & entry.modifierMask() & Qt::AltModifier;
        const bool wantsMetaModifier = entry.modifiers() & entry.modifierMask() & Qt::MetaModifier;
        const bool wantsAnyModifier = entry.state() & entry.stateMask() & KeyStateAnyModifier;

        if ((modifiers & Qt::AltModifier) && !(wantsAltModifier || wantsAnyModifier)
                && !event->text().isEmpty()) {
            textToSend.prepend("\033");
        }
        if ((modifiers & Qt::MetaModifier) && !(wantsMetaModifier || wantsAnyModifier)
                && !event->text().isEmpty()) {
            textToSend.prepend("\030@s");
        }

        if (entry.command() != KeyCmdNone) {
            if (entry.command() & KeyCmdErase)
                textToSend += eraseChar();
            // TODO 其余命令（滚动等）由视图处理
        } else if (!entry.text().isEmpty()) {
            textToSend += m_codec->fromUnicode(QString::fromUtf8(entry.text(true, modifiers)));
        } else if ((modifiers & Qt::ControlModifier) && event->key() >= 0x40 && event->key() < 0x5f) {
            textToSend += (event->key() & 0x1f);
        } else if (event->key() == Qt::Key_Tab) {
            textToSend += 0x09;
        } else if (event->key() == Qt::Key_PageUp) {
            textToSend += "\033[5~";
        } else if (event->key() == Qt::Key_PageDown) {
            textToSend += "\033[6~";
        } else {
            textToSend += m_codec->fromUnicode(event->text());
        }

        emit dataToSend(textToSend.constData(), textToSend.length(), m_codec);
    } else {
        const QString translatorError = tr("No keyboard translator available.  "
                                          "The information needed to convert key presses "
                                          "into characters to send to the terminal "
                                          "is missing.");
        reset();
        feedData(translatorError.toUtf8().constData(), translatorError.count(), false);
    }
}

char DTerminalParser::eraseChar() const
{
    if (m_keyTranslator == nullptr)
        return '\b';
    const DKeyEntry entry = m_keyTranslator->findEntry(Qt::Key_Backspace, Qt::NoModifier, KeyStateNone);
    if (!entry.text().isEmpty())
        return entry.text().at(0);
    return '\b';
}

void DTerminalParser::setBackspaceSequence(char *key, int length)
{
    if (m_keyTranslator == nullptr)
        return;
    const DKeyEntry oldEntry = m_keyTranslator->findEntry(Qt::Key_Backspace, Qt::NoModifier, KeyStateNone);

    DKeyEntry newEntry;
    newEntry.setKeyCode(Qt::Key_Backspace);
    newEntry.setText(QByteArray(key, length));
    m_keyTranslator->replaceEntry(oldEntry, newEntry);
}

void DTerminalParser::setDeleteSequence(char *key, int length)
{
    if (m_keyTranslator == nullptr)
        return;
    const DKeyEntry oldEntry = m_keyTranslator->findEntry(Qt::Key_Delete, Qt::NoModifier, KeyStateNone);

    DKeyEntry newEntry;
    newEntry.setKeyCode(Qt::Key_Delete);
    newEntry.setText(QByteArray(key, length));
    m_keyTranslator->replaceEntry(oldEntry, newEntry);
}

void DTerminalParser::setKeyTranslator(DKeyboardTranslator *translator)
{
    m_keyTranslator = translator;
}

// ---- 字符集 ----

wchar_t DTerminalParser::applyCharset(wchar_t c)
{
    const CharsetState &charset = m_charset[m_currentScreenIndex];
    if (charset.graphic && 0x5f <= c && c <= 0x7e)
        return kVt100Graphics[c - 0x5f];
    if (charset.pound && c == '#')
        return 0xa3;  // 过时模式
    return c;
}

void DTerminalParser::resetCharset(int screenIndex)
{
    CharsetState &charset = m_charset[screenIndex];
    charset.active = 0;
    charset.charset[0] = charset.charset[1] = charset.charset[2] = charset.charset[3] = 'B';
    charset.savedGraphic = false;
    charset.savedPound = false;
    charset.graphic = false;
    charset.pound = false;
}

void DTerminalParser::setCharset(int n, int cs)
{
    m_charset[0].charset[n & 3] = static_cast<char>(cs);
    useCharset(m_charset[0].active);
    m_charset[1].charset[n & 3] = static_cast<char>(cs);
    useCharset(m_charset[1].active);
}

void DTerminalParser::setAndUseCharset(int n, int cs)
{
    CharsetState &charset = m_charset[m_currentScreenIndex];
    charset.charset[n & 3] = static_cast<char>(cs);
    useCharset(n & 3);
}

void DTerminalParser::useCharset(int n)
{
    CharsetState &charset = m_charset[m_currentScreenIndex];
    charset.active = n & 3;
    charset.graphic = (charset.charset[n & 3] == '0');
    charset.pound = (charset.charset[n & 3] == 'A');  // 过时模式
}

void DTerminalParser::saveCursorState()
{
    CharsetState &charset = m_charset[m_currentScreenIndex];
    charset.savedGraphic = charset.graphic;
    charset.savedPound = charset.pound;
    currentScreen()->saveCursor();
}

void DTerminalParser::restoreCursorState()
{
    CharsetState &charset = m_charset[m_currentScreenIndex];
    charset.graphic = charset.savedGraphic;
    charset.pound = charset.savedPound;
    currentScreen()->restoreCursor();
}

// ---- 模式 ----

void DTerminalParser::resetParserModes()
{
    // MODE_Allow132Columns 不重置（对齐 xterm 行为）
    resetParserMode(ParserMode132Columns);
    saveParserMode(ParserMode132Columns);
    resetParserMode(ParserModeMouse1000);
    saveParserMode(ParserModeMouse1000);
    resetParserMode(ParserModeMouse1001);
    saveParserMode(ParserModeMouse1001);
    resetParserMode(ParserModeMouse1002);
    saveParserMode(ParserModeMouse1002);
    resetParserMode(ParserModeMouse1003);
    saveParserMode(ParserModeMouse1003);
    resetParserMode(ParserModeMouse1005);
    saveParserMode(ParserModeMouse1005);
    resetParserMode(ParserModeMouse1006);
    saveParserMode(ParserModeMouse1006);
    resetParserMode(ParserModeMouse1015);
    saveParserMode(ParserModeMouse1015);
    resetParserMode(ParserModeBracketedPaste);
    saveParserMode(ParserModeBracketedPaste);

    resetParserMode(ParserModeAppScreen);
    saveParserMode(ParserModeAppScreen);
    resetParserMode(ParserModeAppCursorKeys);
    saveParserMode(ParserModeAppCursorKeys);
    resetParserMode(ParserModeAppKeypad);
    saveParserMode(ParserModeAppKeypad);
    m_screen[0]->resetMode(ModeNewLine);
    m_screen[1]->resetMode(ModeNewLine);
    setParserMode(ParserModeAnsi);
}

void DTerminalParser::setParserMode(DTerminalParserMode mode)
{
    m_modes[mode] = true;
    switch (mode) {
    case ParserMode132Columns:
        if (getParserMode(ParserModeAllow132Columns))
            clearScreenAndSetColumns(132);
        else
            m_modes[mode] = false;
        break;
    case ParserModeMouse1000:
    case ParserModeMouse1001:
    case ParserModeMouse1002:
    case ParserModeMouse1003:
        // 保留历史实现的行为约定
        emit programUsesMouseChanged(false);
        break;
    case ParserModeMouse1007:
        emit alternateScrollingEnabled(true);
        break;
    case ParserModeBracketedPaste:
        emit programBracketedPasteModeChanged(true);
        break;
    case ParserModeAppScreen:
        m_screen[1]->clearSelection();
        activateScreen(1);
        break;
    default:
        break;
    }
}

void DTerminalParser::resetParserMode(DTerminalParserMode mode)
{
    m_modes[mode] = false;
    switch (mode) {
    case ParserMode132Columns:
        if (getParserMode(ParserModeAllow132Columns))
            clearScreenAndSetColumns(80);
        break;
    case ParserModeMouse1000:
    case ParserModeMouse1001:
    case ParserModeMouse1002:
    case ParserModeMouse1003:
        // 保留历史实现的行为约定
        emit programUsesMouseChanged(true);
        break;
    case ParserModeMouse1007:
        emit alternateScrollingEnabled(false);
        break;
    case ParserModeBracketedPaste:
        emit programBracketedPasteModeChanged(false);
        break;
    case ParserModeAppScreen:
        m_screen[0]->clearSelection();
        activateScreen(0);
        break;
    default:
        break;
    }
}

void DTerminalParser::saveParserMode(DTerminalParserMode mode)
{
    m_savedModes[mode] = m_modes[mode];
}

void DTerminalParser::restoreParserMode(DTerminalParserMode mode)
{
    if (m_savedModes[mode])
        setParserMode(mode);
    else
        resetParserMode(mode);
}

bool DTerminalParser::getParserMode(DTerminalParserMode mode) const
{
    return m_modes[mode];
}

// ---- 屏幕与生命周期 ----

void DTerminalParser::activateScreen(int index)
{
    const DTerminalScreen *old = m_screen[m_currentScreenIndex];
    m_currentScreenIndex = index & 1;
    if (m_screen[m_currentScreenIndex] != old)
        checkScreenInUse();
}

void DTerminalParser::checkScreenInUse()
{
    emit primaryScreenInUse(m_currentScreenIndex == 0);
}

void DTerminalParser::clearHistory()
{
    primaryScreen()->clearHistoryContent();
}

void DTerminalParser::setHistoryStorage(DTerminalHistory::StorageKind kind, int maxLines, bool copyContent)
{
    primaryScreen()->setHistoryStorage(kind, maxLines, copyContent);
    showBulk();
}

void DTerminalParser::setImageSize(int lines, int columns)
{
    if (lines < 1 || columns < 1)
        return;

    const QSize size0(primaryScreen()->columnCount(), primaryScreen()->lineCount());
    const QSize size1(alternateScreen()->columnCount(), alternateScreen()->lineCount());
    const QSize newSize(columns, lines);

    if (newSize == size0 && newSize == size1)
        return;

    primaryScreen()->resizeImage(lines, columns);
    alternateScreen()->resizeImage(lines, columns);

    emit imageSizeChanged(lines, columns);
    bufferedUpdate();
}

QSize DTerminalParser::imageSize() const
{
    return QSize(currentScreen()->columnCount(), currentScreen()->lineCount());
}

int DTerminalParser::lineCount() const
{
    return currentScreen()->lineCount() + currentScreen()->historyLineCount();
}

int DTerminalParser::columnCount() const
{
    return currentScreen()->columnCount();
}

void DTerminalParser::clearEntireScreen()
{
    currentScreen()->clearEntireScreen();
    bufferedUpdate();
}

void DTerminalParser::reset()
{
    resetTokenizer();
    resetParserModes();
    resetCharset(0);
    m_screen[0]->reset();
    resetCharset(1);
    m_screen[1]->reset();
    setCodec(QTextCodec::codecForLocale());

    bufferedUpdate();
}

void DTerminalParser::clearScreenAndSetColumns(int columnCount)
{
    setImageSize(currentScreen()->lineCount(), columnCount);
    clearEntireScreen();
    setDefaultMargins();
    currentScreen()->setCursorPosition(0, 0);
}

void DTerminalParser::setMargins(int top, int bottom)
{
    m_screen[0]->setMargins(top, bottom);
    m_screen[1]->setMargins(top, bottom);
}

void DTerminalParser::setDefaultMargins()
{
    m_screen[0]->setDefaultMargins();
    m_screen[1]->setDefaultMargins();
}

// ---- 编解码 ----

bool DTerminalParser::isUtf8() const
{
    Q_ASSERT(m_codec);
    return m_codec->mibEnum() == 106;
}

void DTerminalParser::setCodec(QTextCodec *codec)
{
    if (codec != nullptr)
        m_codec = codec;
    else
        m_codec = QTextCodec::codecForLocale();

    delete m_decoder;
    m_decoder = m_codec->makeDecoder();

    emit useUtf8Request(isUtf8());
}

bool DTerminalParser::detectIconv2005Standard()
{
    iconv_t handle = iconv_open("UTF-8", "GB18030");
    if (handle == reinterpret_cast<iconv_t>(-1))
        return true;

    QByteArray input("\xFE\x51");
    QByteArray output(input.size() * 2, 0);
    char *inputData = input.data();
    char *outputData = output.data();
    size_t inputLen = static_cast<size_t>(input.count());
    size_t outputLen = static_cast<size_t>(output.count());

    const size_t ret = iconv(handle, &inputData, &inputLen, &outputData, &outputLen);
    iconv_close(handle);

    if (ret == static_cast<size_t>(-1))
        return true;

    // 2005 标准 0xFE51 → U+20087；2022 标准 → U+E816
    if (!output.contains("\uE816")) {
        qInfo() << "Current iconv gb18030 standard is 2005.";
        return true;
    }

    qInfo() << "Current iconv gb18030 standard is 2022.";
    return false;
}

void DTerminalParser::feedData(const char *text, int length, bool isCommandExec)
{
    emit activityChanged(ActivityBusy);
    bufferedUpdate();

    QString utf16Text;
    static int iconv2005 = -1;

    if (QString(m_codec->name()).toUpper().startsWith(QLatin1String("GB")) && !isCommandExec) {
        // GB 系编码：先按 UTF-8 解出再按目标编码重编码解码
        delete m_decoder;
        m_decoder = QTextCodec::codecForName("UTF-8")->makeDecoder();
        utf16Text = m_decoder->toUnicode(text, length);

        QTextCodec *gbk = QTextCodec::codecForName(m_codec->name());
        const QByteArray gbkBytes = gbk->fromUnicode(utf16Text);

        delete m_decoder;
        m_decoder = m_codec->makeDecoder();
        utf16Text = m_decoder->toUnicode(gbkBytes);
    } else {
        if (iconv2005 == -1) {
            iconv2005 = detectIconv2005Standard() ? 1 : 0;
            qInfo() << "Is Used 2005 standard's gb18030 iconv?" << iconv2005;
        }
        if (m_codec->name().toUpper().contains(QLatin1String("GB18030")) && iconv2005 == 1
                && m_encodingConverter) {
            // 2005 标准下经编码转换器修正
            const QByteArray raw(text, length);
            utf16Text = QString(m_encodingConverter(raw, QStringLiteral("GB18030"), QStringLiteral("UTF-8")));
        } else {
            utf16Text = m_decoder->toUnicode(text, length);
        }
    }

    // bash 长提示符光标修正：去掉提示符尾部的五个退格
    if (utf16Text.startsWith(QStringLiteral("\u001B]0;"))
            && utf16Text.endsWith(QStringLiteral("\b\b\b\b\b"))) {
        if (m_foregroundNameProvider
                && m_foregroundNameProvider(m_sessionId) == QLatin1String("bash")) {
            utf16Text.replace(QStringLiteral("\b\b\b\b\b"), QString());
        }
    }

    const std::wstring unicodeText = utf16Text.toStdWString();
    for (wchar_t c : unicodeText)
        receiveChar(c);

    // zmodem 上传指示
    for (int i = 0; i < length; ++i) {
        if (text[i] == '\030'
                && (length - i - 1 > 3)
                && (std::strncmp(text + i + 1, "B00", 3) == 0)) {
            emit zmodemDetected();
        }
    }
}

// ---- 刷新节流 ----

void DTerminalParser::bufferedUpdate()
{
    m_bulkTimer1->start(10);
    if (!m_bulkTimer2->isActive())
        m_bulkTimer2->start(40);
}

void DTerminalParser::showBulk()
{
    m_bulkTimer1->stop();
    m_bulkTimer2->stop();

    emit outputChanged();

    m_screen[0]->resetScrolledLines();
    m_screen[1]->resetScrolledLines();
    m_screen[0]->resetDroppedLines();
    m_screen[1]->resetDroppedLines();
}

// ---- 指令执行 ----

void DTerminalParser::processToken(int token, wchar_t p, int q)
{
    DTerminalScreen *screen = currentScreen();

    switch (token) {
    case makeToken(TokenChar, 0, 0):
        screen->displayCharacter(p);
        break;

    // ---- 控制字符 ----
    case makeToken(TokenControl, '@', 0):   /* NUL */ break;
    case makeToken(TokenControl, 'A', 0):   /* SOH */ break;
    case makeToken(TokenControl, 'B', 0):   /* STX */ break;
    case makeToken(TokenControl, 'C', 0):   /* ETX */ break;
    case makeToken(TokenControl, 'D', 0):   /* EOT */ break;
    case makeToken(TokenControl, 'E', 0):
        reportAnswerBack();
        break;
    case makeToken(TokenControl, 'F', 0):   /* ACK */ break;
    case makeToken(TokenControl, 'G', 0):
        emit activityChanged(ActivityBell);
        break;
    case makeToken(TokenControl, 'H', 0):
        screen->backspace();
        break;
    case makeToken(TokenControl, 'I', 0):
        screen->tab();
        break;
    case makeToken(TokenControl, 'J', 0):
    case makeToken(TokenControl, 'K', 0):
    case makeToken(TokenControl, 'L', 0):
        screen->newLine();
        break;
    case makeToken(TokenControl, 'M', 0):
        screen->toStartOfLine();
        break;
    case makeToken(TokenControl, 'N', 0):
        useCharset(1);
        break;
    case makeToken(TokenControl, 'O', 0):
        useCharset(0);
        break;
    case makeToken(TokenControl, 'P', 0):   /* DLE */ break;
    case makeToken(TokenControl, 'Q', 0):   /* DC1 */ break;
    case makeToken(TokenControl, 'R', 0):   /* DC2 */ break;
    case makeToken(TokenControl, 'S', 0):   /* DC3 */ break;
    case makeToken(TokenControl, 'T', 0):   /* DC4 */ break;
    case makeToken(TokenControl, 'U', 0):   /* NAK */ break;
    case makeToken(TokenControl, 'V', 0):   /* SYN */ break;
    case makeToken(TokenControl, 'W', 0):   /* ETB */ break;
    case makeToken(TokenControl, 'X', 0):
        screen->displayCharacter(0x2592);
        break;
    case makeToken(TokenControl, 'Y', 0):   /* EM */ break;
    case makeToken(TokenControl, 'Z', 0):
        screen->displayCharacter(0x2592);
        break;
    case makeToken(TokenControl, '[', 0):   /* ESC 不在此处理 */ break;
    case makeToken(TokenControl, '\\', 0):  /* FS */ break;
    case makeToken(TokenControl, ']', 0):   /* GS */ break;
    case makeToken(TokenControl, '^', 0):   /* RS */ break;
    case makeToken(TokenControl, '_', 0):   /* US */ break;

    // ---- ESC 序列 ----
    case makeToken(TokenEscape, 'D', 0):
        screen->index();
        break;
    case makeToken(TokenEscape, 'E', 0):
        screen->nextLine();
        break;
    case makeToken(TokenEscape, 'H', 0):
        screen->changeTabStop(true);
        break;
    case makeToken(TokenEscape, 'M', 0):
        screen->reverseIndex();
        break;
    case makeToken(TokenEscape, 'Z', 0):
        reportTerminalType();
        break;
    case makeToken(TokenEscape, 'c', 0):
        reset();
        break;
    case makeToken(TokenEscape, 'n', 0):
        useCharset(2);
        break;
    case makeToken(TokenEscape, 'o', 0):
        useCharset(3);
        break;
    case makeToken(TokenEscape, '7', 0):
        saveCursorState();
        break;
    case makeToken(TokenEscape, '8', 0):
        restoreCursorState();
        break;
    case makeToken(TokenEscape, '=', 0):
        setParserMode(ParserModeAppKeypad);
        break;
    case makeToken(TokenEscape, '>', 0):
        resetParserMode(ParserModeAppKeypad);
        break;
    case makeToken(TokenEscape, '<', 0):
        setParserMode(ParserModeAnsi);
        break;

    // ---- 字符集选择 ESC ( / ) / * / + + 字符 ----
    case makeToken(TokenEscapeCharset, '(', '0'): setCharset(0, '0'); break;
    case makeToken(TokenEscapeCharset, '(', 'A'): setCharset(0, 'A'); break;
    case makeToken(TokenEscapeCharset, '(', 'B'): setCharset(0, 'B'); break;
    case makeToken(TokenEscapeCharset, ')', '0'): setCharset(1, '0'); break;
    case makeToken(TokenEscapeCharset, ')', 'A'): setCharset(1, 'A'); break;
    case makeToken(TokenEscapeCharset, ')', 'B'): setCharset(1, 'B'); break;
    case makeToken(TokenEscapeCharset, '*', '0'): setCharset(2, '0'); break;
    case makeToken(TokenEscapeCharset, '*', 'A'): setCharset(2, 'A'); break;
    case makeToken(TokenEscapeCharset, '*', 'B'): setCharset(2, 'B'); break;
    case makeToken(TokenEscapeCharset, '+', '0'): setCharset(3, '0'); break;
    case makeToken(TokenEscapeCharset, '+', 'A'): setCharset(3, 'A'); break;
    case makeToken(TokenEscapeCharset, '+', 'B'): setCharset(3, 'B'); break;
    case makeToken(TokenEscapeCharset, '%', 'G'):
        setCodec(QTextCodec::codecForName("utf8"));
        break;
    case makeToken(TokenEscapeCharset, '%', '@'):
        setCodec(QTextCodec::codecForLocale());
        break;

    // ---- ESC # 序列 ----
    case makeToken(TokenEscapeHash, '3', 0):  // 双高行上半
    case makeToken(TokenEscapeHash, '4', 0):  // 双高行下半
        screen->setLineFlag(LineDoubleWidth, true);
        screen->setLineFlag(LineDoubleHeightTop, true);
        break;
    case makeToken(TokenEscapeHash, '5', 0):  // 单宽单高
        screen->setLineFlag(LineDoubleWidth, false);
        screen->setLineFlag(LineDoubleHeightTop, false);
        break;
    case makeToken(TokenEscapeHash, '6', 0):  // 双宽单高
        screen->setLineFlag(LineDoubleWidth, true);
        screen->setLineFlag(LineDoubleHeightTop, false);
        break;
    case makeToken(TokenEscapeHash, '8', 0):
        screen->helpAlign();
        break;

    // ---- CSI 参数指令 ----
    case makeToken(TokenCsiParam, 't', 8):  // resize = \e[8;<row>;<col>t
        setImageSize(p /*lines*/, q /*columns*/);
        emit imageResizeRequest(QSize(q, p));
        break;
    case makeToken(TokenCsiParam, 't', 28):
        emit changeTabTextColorRequest(p);
        break;

    case makeToken(TokenCsiParam, 'K', 0):
        screen->clearToEndOfLine();
        break;
    case makeToken(TokenCsiParam, 'K', 1):
        screen->clearToBeginOfLine();
        break;
    case makeToken(TokenCsiParam, 'K', 2):
        screen->clearEntireLine();
        break;
    case makeToken(TokenCsiParam, 'J', 0):
        screen->clearToEndOfScreen();
        break;
    case makeToken(TokenCsiParam, 'J', 1):
        screen->clearToBeginOfScreen();
        break;
    case makeToken(TokenCsiParam, 'J', 2):
        screen->clearEntireScreen();
        break;
    case makeToken(TokenCsiParam, 'J', 3):
        clearHistory();
        break;
    case makeToken(TokenCsiParam, 'g', 0):
        screen->changeTabStop(false);
        break;
    case makeToken(TokenCsiParam, 'g', 3):
        screen->clearTabStops();
        break;
    case makeToken(TokenCsiParam, 'h', 4):
        screen->setMode(ModeInsert);
        break;
    case makeToken(TokenCsiParam, 'h', 20):
        m_screen[0]->setMode(ModeNewLine);
        m_screen[1]->setMode(ModeNewLine);
        break;
    case makeToken(TokenCsiParam, 'i', 0):   /* 打印机：忽略 */ break;
    case makeToken(TokenCsiParam, 'l', 4):
        screen->resetMode(ModeInsert);
        break;
    case makeToken(TokenCsiParam, 'l', 20):
        m_screen[0]->resetMode(ModeNewLine);
        m_screen[1]->resetMode(ModeNewLine);
        break;
    case makeToken(TokenCsiParam, 's', 0):
        saveCursorState();
        break;
    case makeToken(TokenCsiParam, 'u', 0):
        restoreCursorState();
        break;

    // ---- SGR ----
    case makeToken(TokenCsiParam, 'm', 0):
        screen->setDefaultRendition();
        break;
    case makeToken(TokenCsiParam, 'm', 1):
        screen->setRendition(StyleBold);
        break;
    case makeToken(TokenCsiParam, 'm', 2):
        screen->setRendition(StyleFaint);
        break;
    case makeToken(TokenCsiParam, 'm', 3):
        screen->setRendition(StyleItalic);
        break;
    case makeToken(TokenCsiParam, 'm', 4):
        screen->setRendition(StyleUnderline);
        break;
    case makeToken(TokenCsiParam, 'm', 5):
        screen->setRendition(StyleBlink);
        break;
    case makeToken(TokenCsiParam, 'm', 7):
        screen->setRendition(StyleReverse);
        break;
    case makeToken(TokenCsiParam, 'm', 8):
        screen->setRendition(StyleConceal);
        break;
    case makeToken(TokenCsiParam, 'm', 9):
        screen->setRendition(StyleStrikeout);
        break;
    case makeToken(TokenCsiParam, 'm', 53):
        screen->setRendition(StyleOverline);
        break;
    case makeToken(TokenCsiParam, 'm', 10):  /* 字体映射：忽略 */ break;
    case makeToken(TokenCsiParam, 'm', 11):  /* 字体映射：忽略 */ break;
    case makeToken(TokenCsiParam, 'm', 12):  /* 字体映射：忽略 */ break;
    case makeToken(TokenCsiParam, 'm', 21):
        screen->resetRendition(StyleBold);
        break;
    case makeToken(TokenCsiParam, 'm', 22):
        screen->resetRendition(StyleBold);
        screen->resetRendition(StyleFaint);
        break;
    case makeToken(TokenCsiParam, 'm', 23):
        screen->resetRendition(StyleItalic);
        break;
    case makeToken(TokenCsiParam, 'm', 24):
        screen->resetRendition(StyleUnderline);
        break;
    case makeToken(TokenCsiParam, 'm', 25):
        screen->resetRendition(StyleBlink);
        break;
    case makeToken(TokenCsiParam, 'm', 27):
        screen->resetRendition(StyleReverse);
        break;
    case makeToken(TokenCsiParam, 'm', 28):
        screen->resetRendition(StyleConceal);
        break;
    case makeToken(TokenCsiParam, 'm', 29):
        screen->resetRendition(StyleStrikeout);
        break;
    case makeToken(TokenCsiParam, 'm', 55):
        screen->resetRendition(StyleOverline);
        break;

    case makeToken(TokenCsiParam, 'm', 30): screen->setForeColor(ColorSpaceSystem, 0); break;
    case makeToken(TokenCsiParam, 'm', 31): screen->setForeColor(ColorSpaceSystem, 1); break;
    case makeToken(TokenCsiParam, 'm', 32): screen->setForeColor(ColorSpaceSystem, 2); break;
    case makeToken(TokenCsiParam, 'm', 33): screen->setForeColor(ColorSpaceSystem, 3); break;
    case makeToken(TokenCsiParam, 'm', 34): screen->setForeColor(ColorSpaceSystem, 4); break;
    case makeToken(TokenCsiParam, 'm', 35): screen->setForeColor(ColorSpaceSystem, 5); break;
    case makeToken(TokenCsiParam, 'm', 36): screen->setForeColor(ColorSpaceSystem, 6); break;
    case makeToken(TokenCsiParam, 'm', 37): screen->setForeColor(ColorSpaceSystem, 7); break;
    case makeToken(TokenCsiParam, 'm', 38): screen->setForeColor(p, q); break;
    case makeToken(TokenCsiParam, 'm', 39): screen->setForeColor(ColorSpaceDefault, 0); break;

    case makeToken(TokenCsiParam, 'm', 40): screen->setBackColor(ColorSpaceSystem, 0); break;
    case makeToken(TokenCsiParam, 'm', 41): screen->setBackColor(ColorSpaceSystem, 1); break;
    case makeToken(TokenCsiParam, 'm', 42): screen->setBackColor(ColorSpaceSystem, 2); break;
    case makeToken(TokenCsiParam, 'm', 43): screen->setBackColor(ColorSpaceSystem, 3); break;
    case makeToken(TokenCsiParam, 'm', 44): screen->setBackColor(ColorSpaceSystem, 4); break;
    case makeToken(TokenCsiParam, 'm', 45): screen->setBackColor(ColorSpaceSystem, 5); break;
    case makeToken(TokenCsiParam, 'm', 46): screen->setBackColor(ColorSpaceSystem, 6); break;
    case makeToken(TokenCsiParam, 'm', 47): screen->setBackColor(ColorSpaceSystem, 7); break;
    case makeToken(TokenCsiParam, 'm', 48): screen->setBackColor(p, q); break;
    case makeToken(TokenCsiParam, 'm', 49): screen->setBackColor(ColorSpaceDefault, 1); break;

    case makeToken(TokenCsiParam, 'm', 90): screen->setForeColor(ColorSpaceSystem, 8); break;
    case makeToken(TokenCsiParam, 'm', 91): screen->setForeColor(ColorSpaceSystem, 9); break;
    case makeToken(TokenCsiParam, 'm', 92): screen->setForeColor(ColorSpaceSystem, 10); break;
    case makeToken(TokenCsiParam, 'm', 93): screen->setForeColor(ColorSpaceSystem, 11); break;
    case makeToken(TokenCsiParam, 'm', 94): screen->setForeColor(ColorSpaceSystem, 12); break;
    case makeToken(TokenCsiParam, 'm', 95): screen->setForeColor(ColorSpaceSystem, 13); break;
    case makeToken(TokenCsiParam, 'm', 96): screen->setForeColor(ColorSpaceSystem, 14); break;
    case makeToken(TokenCsiParam, 'm', 97): screen->setForeColor(ColorSpaceSystem, 15); break;

    case makeToken(TokenCsiParam, 'm', 100): screen->setBackColor(ColorSpaceSystem, 8); break;
    case makeToken(TokenCsiParam, 'm', 101): screen->setBackColor(ColorSpaceSystem, 9); break;
    case makeToken(TokenCsiParam, 'm', 102): screen->setBackColor(ColorSpaceSystem, 10); break;
    case makeToken(TokenCsiParam, 'm', 103): screen->setBackColor(ColorSpaceSystem, 11); break;
    case makeToken(TokenCsiParam, 'm', 104): screen->setBackColor(ColorSpaceSystem, 12); break;
    case makeToken(TokenCsiParam, 'm', 105): screen->setBackColor(ColorSpaceSystem, 13); break;
    case makeToken(TokenCsiParam, 'm', 106): screen->setBackColor(ColorSpaceSystem, 14); break;
    case makeToken(TokenCsiParam, 'm', 107): screen->setBackColor(ColorSpaceSystem, 15); break;

    case makeToken(TokenCsiParam, 'n', 5):
        reportStatus();
        break;
    case makeToken(TokenCsiParam, 'n', 6):
        reportCursorPosition();
        break;
    case makeToken(TokenCsiParam, 'q', 0):   /* LED 关 */ break;
    case makeToken(TokenCsiParam, 'q', 1):   /* LED1 */ break;
    case makeToken(TokenCsiParam, 'q', 2):   /* LED2 */ break;
    case makeToken(TokenCsiParam, 'q', 3):   /* LED3 */ break;
    case makeToken(TokenCsiParam, 'q', 4):   /* LED4 */ break;
    case makeToken(TokenCsiParam, 'x', 0):
        reportTerminalParms(2);
        break;
    case makeToken(TokenCsiParam, 'x', 1):
        reportTerminalParms(3);
        break;

    // DECRQM ANSI 模式查询：CSI Pd $ p
    case makeToken(TokenCsiParam, 'p', 2):
        reportAnsiMode(2, 2);
        break;
    case makeToken(TokenCsiParam, 'p', 4):
        reportAnsiMode(4, screen->getMode(ModeInsert) ? 1 : 2);
        break;
    case makeToken(TokenCsiParam, 'p', 10):
        reportAnsiMode(10, 4);
        break;
    case makeToken(TokenCsiParam, 'p', 20):
        reportAnsiMode(20, currentScreen()->getMode(ModeNewLine) ? 1 : 2);
        break;

    // ---- DECSCUSR：CSI Ps SP q ----
    case makeToken(TokenCsiParamSpace, 'q', 0):
    case makeToken(TokenCsiParamSpace, 'q', 1):
        emit cursorChanged(0 /*Block*/, true);
        break;
    case makeToken(TokenCsiParamSpace, 'q', 2):
        emit cursorChanged(0 /*Block*/, false);
        break;
    case makeToken(TokenCsiParamSpace, 'q', 3):
        emit cursorChanged(1 /*Underline*/, true);
        break;
    case makeToken(TokenCsiParamSpace, 'q', 4):
        emit cursorChanged(1 /*Underline*/, false);
        break;
    case makeToken(TokenCsiParamSpace, 'q', 5):
        emit cursorChanged(2 /*IBeam*/, true);
        break;
    case makeToken(TokenCsiParamSpace, 'q', 6):
        emit cursorChanged(2 /*IBeam*/, false);
        break;
    case makeToken(TokenCsiParamSpace, 'q', 7):
        emit cursorChanged(3 /*BoldUnderline*/, true);
        break;
    case makeToken(TokenCsiParamSpace, 'q', 8):
        emit cursorChanged(3 /*BoldUnderline*/, false);
        break;

    // ---- CSI 单参指令 ----
    case makeToken(TokenCsiPlain, '@', 0): screen->insertChars(p); break;
    case makeToken(TokenCsiPlain, 'A', 0): screen->cursorUp(p); break;
    case makeToken(TokenCsiPlain, 'B', 0): screen->cursorDown(p); break;
    case makeToken(TokenCsiPlain, 'C', 0): screen->cursorRight(p); break;
    case makeToken(TokenCsiPlain, 'D', 0): screen->cursorLeft(p); break;
    case makeToken(TokenCsiPlain, 'E', 0): /* 光标下移 p 行：未实现 */ break;
    case makeToken(TokenCsiPlain, 'F', 0): /* 光标上移 p 行：未实现 */ break;
    case makeToken(TokenCsiPlain, 'G', 0): screen->setCursorColumn(p); break;
    case makeToken(TokenCsiPlain, 'H', 0): screen->setCursorPosition(p, q); break;
    case makeToken(TokenCsiPlain, 'I', 0): screen->tab(p); break;
    case makeToken(TokenCsiPlain, 'L', 0): screen->insertLines(p); break;
    case makeToken(TokenCsiPlain, 'M', 0): screen->deleteLines(p); break;
    case makeToken(TokenCsiPlain, 'P', 0): screen->deleteChars(p); break;
    case makeToken(TokenCsiPlain, 'S', 0): screen->scrollUp(p); break;
    case makeToken(TokenCsiPlain, 'T', 0): screen->scrollDown(p); break;
    case makeToken(TokenCsiPlain, 'X', 0): screen->eraseChars(p); break;
    case makeToken(TokenCsiPlain, 'Z', 0): screen->backtab(p); break;
    case makeToken(TokenCsiPlain, 'b', 0): screen->repeatChars(p); break;
    case makeToken(TokenCsiPlain, 'c', 0): reportTerminalType(); break;
    case makeToken(TokenCsiPlain, 'd', 0): screen->setCursorRow(p); break;
    case makeToken(TokenCsiPlain, 'f', 0): screen->setCursorPosition(p, q); break;
    case makeToken(TokenCsiPlain, 'r', 0): setMargins(p, q); break;
    case makeToken(TokenCsiPlain, 'y', 0): /* 置信测试：忽略 */ break;

    // ---- DEC 私有模式 ----
    case makeToken(TokenCsiPrivate, 'h', 1): setParserMode(ParserModeAppCursorKeys); break;
    case makeToken(TokenCsiPrivate, 'l', 1): resetParserMode(ParserModeAppCursorKeys); break;
    case makeToken(TokenCsiPrivate, 's', 1): saveParserMode(ParserModeAppCursorKeys); break;
    case makeToken(TokenCsiPrivate, 'r', 1): restoreParserMode(ParserModeAppCursorKeys); break;

    case makeToken(TokenCsiPrivate, 'l', 2): resetParserMode(ParserModeAnsi); break;

    case makeToken(TokenCsiPrivate, 'h', 3): setParserMode(ParserMode132Columns); break;
    case makeToken(TokenCsiPrivate, 'l', 3): resetParserMode(ParserMode132Columns); break;

    case makeToken(TokenCsiPrivate, 'h', 4): /* 平滑滚动：忽略 */ break;
    case makeToken(TokenCsiPrivate, 'l', 4): /* 平滑滚动：忽略 */ break;

    case makeToken(TokenCsiPrivate, 'h', 5): screen->setMode(ModeInverseVideo); break;
    case makeToken(TokenCsiPrivate, 'l', 5): screen->resetMode(ModeInverseVideo); break;

    case makeToken(TokenCsiPrivate, 'h', 6): screen->setMode(ModeOrigin); break;
    case makeToken(TokenCsiPrivate, 'l', 6): screen->resetMode(ModeOrigin); break;
    case makeToken(TokenCsiPrivate, 's', 6): screen->saveMode(ModeOrigin); break;
    case makeToken(TokenCsiPrivate, 'r', 6): screen->restoreMode(ModeOrigin); break;

    case makeToken(TokenCsiPrivate, 'h', 7): screen->setMode(ModeWrap); break;
    case makeToken(TokenCsiPrivate, 'l', 7): screen->resetMode(ModeWrap); break;
    case makeToken(TokenCsiPrivate, 's', 7): screen->saveMode(ModeWrap); break;
    case makeToken(TokenCsiPrivate, 'r', 7): screen->restoreMode(ModeWrap); break;

    case makeToken(TokenCsiPrivate, 'h', 8): /* 自动重复：忽略 */ break;
    case makeToken(TokenCsiPrivate, 'l', 8): break;
    case makeToken(TokenCsiPrivate, 's', 8): break;
    case makeToken(TokenCsiPrivate, 'r', 8): break;

    case makeToken(TokenCsiPrivate, 'h', 9): /* 隔行：忽略 */ break;
    case makeToken(TokenCsiPrivate, 'l', 9): break;
    case makeToken(TokenCsiPrivate, 's', 9): break;
    case makeToken(TokenCsiPrivate, 'r', 9): break;

    case makeToken(TokenCsiPrivate, 'h', 12): /* 光标闪烁：忽略 */ break;
    case makeToken(TokenCsiPrivate, 'l', 12): break;
    case makeToken(TokenCsiPrivate, 's', 12): break;
    case makeToken(TokenCsiPrivate, 'r', 12): break;

    case makeToken(TokenCsiPrivate, 'h', 25): screen->setMode(ModeShowCursor); break;
    case makeToken(TokenCsiPrivate, 'l', 25): screen->resetMode(ModeShowCursor); break;
    case makeToken(TokenCsiPrivate, 's', 25): screen->saveMode(ModeShowCursor); break;
    case makeToken(TokenCsiPrivate, 'r', 25): screen->restoreMode(ModeShowCursor); break;

    case makeToken(TokenCsiPrivate, 'h', 40): setParserMode(ParserModeAllow132Columns); break;
    case makeToken(TokenCsiPrivate, 'l', 40): resetParserMode(ParserModeAllow132Columns); break;

    case makeToken(TokenCsiPrivate, 'h', 41): /* 过时：忽略 */ break;
    case makeToken(TokenCsiPrivate, 'l', 41): break;
    case makeToken(TokenCsiPrivate, 's', 41): break;
    case makeToken(TokenCsiPrivate, 'r', 41): break;

    case makeToken(TokenCsiPrivate, 'h', 47): setParserMode(ParserModeAppScreen); break;
    case makeToken(TokenCsiPrivate, 'l', 47): resetParserMode(ParserModeAppScreen); break;
    case makeToken(TokenCsiPrivate, 's', 47): saveParserMode(ParserModeAppScreen); break;
    case makeToken(TokenCsiPrivate, 'r', 47): restoreParserMode(ParserModeAppScreen); break;

    case makeToken(TokenCsiPrivate, 'h', 67): /* DECBKM：忽略 */ break;
    case makeToken(TokenCsiPrivate, 'l', 67): break;
    case makeToken(TokenCsiPrivate, 's', 67): break;
    case makeToken(TokenCsiPrivate, 'r', 67): break;

    case makeToken(TokenCsiPrivate, 'h', 1000): setParserMode(ParserModeMouse1000); break;
    case makeToken(TokenCsiPrivate, 'l', 1000): resetParserMode(ParserModeMouse1000); break;
    case makeToken(TokenCsiPrivate, 's', 1000): saveParserMode(ParserModeMouse1000); break;
    case makeToken(TokenCsiPrivate, 'r', 1000): restoreParserMode(ParserModeMouse1000); break;

    case makeToken(TokenCsiPrivate, 'h', 1001): /* 高亮跟踪：忽略 */ break;
    case makeToken(TokenCsiPrivate, 'l', 1001): resetParserMode(ParserModeMouse1001); break;
    case makeToken(TokenCsiPrivate, 's', 1001): break;
    case makeToken(TokenCsiPrivate, 'r', 1001): break;

    case makeToken(TokenCsiPrivate, 'h', 1002): setParserMode(ParserModeMouse1002); break;
    case makeToken(TokenCsiPrivate, 'l', 1002): resetParserMode(ParserModeMouse1002); break;
    case makeToken(TokenCsiPrivate, 's', 1002): saveParserMode(ParserModeMouse1002); break;
    case makeToken(TokenCsiPrivate, 'r', 1002): restoreParserMode(ParserModeMouse1002); break;

    case makeToken(TokenCsiPrivate, 'h', 1003): setParserMode(ParserModeMouse1003); break;
    case makeToken(TokenCsiPrivate, 'l', 1003): resetParserMode(ParserModeMouse1003); break;
    case makeToken(TokenCsiPrivate, 's', 1003): saveParserMode(ParserModeMouse1003); break;
    case makeToken(TokenCsiPrivate, 'r', 1003): restoreParserMode(ParserModeMouse1003); break;

    case makeToken(TokenCsiPrivate, 'h', 1004): m_reportFocusEvents = true; break;
    case makeToken(TokenCsiPrivate, 'l', 1004): m_reportFocusEvents = false; break;

    case makeToken(TokenCsiPrivate, 'h', 1005): setParserMode(ParserModeMouse1005); break;
    case makeToken(TokenCsiPrivate, 'l', 1005): resetParserMode(ParserModeMouse1005); break;
    case makeToken(TokenCsiPrivate, 's', 1005): saveParserMode(ParserModeMouse1005); break;
    case makeToken(TokenCsiPrivate, 'r', 1005): restoreParserMode(ParserModeMouse1005); break;

    case makeToken(TokenCsiPrivate, 'h', 1006): setParserMode(ParserModeMouse1006); break;
    case makeToken(TokenCsiPrivate, 'l', 1006): resetParserMode(ParserModeMouse1006); break;
    case makeToken(TokenCsiPrivate, 's', 1006): saveParserMode(ParserModeMouse1006); break;
    case makeToken(TokenCsiPrivate, 'r', 1006): restoreParserMode(ParserModeMouse1006); break;

    case makeToken(TokenCsiPrivate, 'h', 1015): setParserMode(ParserModeMouse1015); break;
    case makeToken(TokenCsiPrivate, 'l', 1015): resetParserMode(ParserModeMouse1015); break;
    case makeToken(TokenCsiPrivate, 's', 1015): saveParserMode(ParserModeMouse1015); break;
    case makeToken(TokenCsiPrivate, 'r', 1015): restoreParserMode(ParserModeMouse1015); break;

    case makeToken(TokenCsiPrivate, 'h', 1034): /* 8bit 输入：忽略 */ break;

    case makeToken(TokenCsiPrivate, 'h', 1047): setParserMode(ParserModeAppScreen); break;
    case makeToken(TokenCsiPrivate, 'l', 1047):
        m_screen[1]->clearEntireScreen();
        resetParserMode(ParserModeAppScreen);
        break;
    case makeToken(TokenCsiPrivate, 's', 1047): saveParserMode(ParserModeAppScreen); break;
    case makeToken(TokenCsiPrivate, 'r', 1047): restoreParserMode(ParserModeAppScreen); break;

    case makeToken(TokenCsiPrivate, 'h', 1048): saveCursorState(); break;
    case makeToken(TokenCsiPrivate, 'l', 1048): restoreCursorState(); break;
    case makeToken(TokenCsiPrivate, 's', 1048): saveCursorState(); break;
    case makeToken(TokenCsiPrivate, 'r', 1048): restoreCursorState(); break;

    case makeToken(TokenCsiPrivate, 'h', 1049):
        saveCursorState();
        m_screen[1]->clearEntireScreen();
        setParserMode(ParserModeAppScreen);
        break;
    case makeToken(TokenCsiPrivate, 'l', 1049):
        resetParserMode(ParserModeAppScreen);
        restoreCursorState();
        break;

    case makeToken(TokenCsiPrivate, 'h', 2004): setParserMode(ParserModeBracketedPaste); break;
    case makeToken(TokenCsiPrivate, 'l', 2004): resetParserMode(ParserModeBracketedPaste); break;
    case makeToken(TokenCsiPrivate, 's', 2004): saveParserMode(ParserModeBracketedPaste); break;
    case makeToken(TokenCsiPrivate, 'r', 2004): restoreParserMode(ParserModeBracketedPaste); break;

    // DECRQM DEC 私有模式查询：CSI ? Pd $ p
    case makeToken(TokenCsiPrivate, 'p', 1): reportDecMode(1, getParserMode(ParserModeAppCursorKeys) ? 1 : 2); break;
    case makeToken(TokenCsiPrivate, 'p', 2): reportDecMode(2, getParserMode(ParserModeAnsi) ? 1 : 2); break;
    case makeToken(TokenCsiPrivate, 'p', 3): reportDecMode(3, getParserMode(ParserMode132Columns) ? 1 : 2); break;
    case makeToken(TokenCsiPrivate, 'p', 4): reportDecMode(4, 4); break;
    case makeToken(TokenCsiPrivate, 'p', 5): reportDecMode(5, screen->getMode(ModeInverseVideo) ? 1 : 2); break;
    case makeToken(TokenCsiPrivate, 'p', 6): reportDecMode(6, screen->getMode(ModeOrigin) ? 1 : 2); break;
    case makeToken(TokenCsiPrivate, 'p', 7): reportDecMode(7, screen->getMode(ModeWrap) ? 1 : 2); break;
    case makeToken(TokenCsiPrivate, 'p', 8): reportDecMode(8, 4); break;
    case makeToken(TokenCsiPrivate, 'p', 9): reportDecMode(9, 4); break;
    case makeToken(TokenCsiPrivate, 'p', 10): reportDecMode(10, 4); break;
    case makeToken(TokenCsiPrivate, 'p', 12): reportDecMode(12, 4); break;
    case makeToken(TokenCsiPrivate, 'p', 25): reportDecMode(25, screen->getMode(ModeShowCursor) ? 1 : 2); break;
    case makeToken(TokenCsiPrivate, 'p', 47): reportDecMode(47, getParserMode(ParserModeAppScreen) ? 1 : 2); break;
    case makeToken(TokenCsiPrivate, 'p', 1000): reportDecMode(1000, getParserMode(ParserModeMouse1000) ? 1 : 2); break;
    case makeToken(TokenCsiPrivate, 'p', 1002): reportDecMode(1002, getParserMode(ParserModeMouse1002) ? 1 : 2); break;
    case makeToken(TokenCsiPrivate, 'p', 1003): reportDecMode(1003, getParserMode(ParserModeMouse1003) ? 1 : 2); break;
    case makeToken(TokenCsiPrivate, 'p', 1004): reportDecMode(1004, m_reportFocusEvents ? 1 : 2); break;
    case makeToken(TokenCsiPrivate, 'p', 1005): reportDecMode(1005, getParserMode(ParserModeMouse1005) ? 1 : 2); break;
    case makeToken(TokenCsiPrivate, 'p', 1006): reportDecMode(1006, getParserMode(ParserModeMouse1006) ? 1 : 2); break;
    case makeToken(TokenCsiPrivate, 'p', 1015): reportDecMode(1015, getParserMode(ParserModeMouse1015) ? 1 : 2); break;
    case makeToken(TokenCsiPrivate, 'p', 1034): reportDecMode(1034, 4); break;
    case makeToken(TokenCsiPrivate, 'p', 1047): reportDecMode(1047, getParserMode(ParserModeAppScreen) ? 1 : 2); break;
    case makeToken(TokenCsiPrivate, 'p', 1049): reportDecMode(1049, getParserMode(ParserModeAppScreen) ? 1 : 2); break;
    case makeToken(TokenCsiPrivate, 'p', 2004): reportDecMode(2004, getParserMode(ParserModeBracketedPaste) ? 1 : 2); break;

    // ---- CSI ! 序列 ----
    case makeToken(TokenCsiBang, 'p', 0): /* DEC 复位：忽略 */ break;

    // ---- VT52 ----
    case makeToken(TokenVt52, 'A', 0): screen->cursorUp(1); break;
    case makeToken(TokenVt52, 'B', 0): screen->cursorDown(1); break;
    case makeToken(TokenVt52, 'C', 0): screen->cursorRight(1); break;
    case makeToken(TokenVt52, 'D', 0): screen->cursorLeft(1); break;
    case makeToken(TokenVt52, 'F', 0): setAndUseCharset(0, '0'); break;
    case makeToken(TokenVt52, 'G', 0): setAndUseCharset(0, 'B'); break;
    case makeToken(TokenVt52, 'H', 0): screen->setCursorPosition(1, 1); break;
    case makeToken(TokenVt52, 'I', 0): screen->reverseIndex(); break;
    case makeToken(TokenVt52, 'J', 0): screen->clearToEndOfScreen(); break;
    case makeToken(TokenVt52, 'K', 0): screen->clearToEndOfLine(); break;
    case makeToken(TokenVt52, 'Y', 0): screen->setCursorPosition(p - 31, q - 31); break;
    case makeToken(TokenVt52, 'Z', 0): reportTerminalType(); break;
    case makeToken(TokenVt52, '<', 0): setParserMode(ParserModeAnsi); break;
    case makeToken(TokenVt52, '=', 0): setParserMode(ParserModeAppKeypad); break;
    case makeToken(TokenVt52, '>', 0): resetParserMode(ParserModeAppKeypad); break;

    // ---- CSI > 序列 ----
    case makeToken(TokenCsiGreater, 'c', 0):
        reportSecondaryAttributes();
        break;

    default:
        reportDecodingError();
        break;
    }
}
