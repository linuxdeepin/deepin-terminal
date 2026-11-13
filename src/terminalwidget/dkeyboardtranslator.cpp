// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dkeyboardtranslator.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QKeySequence>
#include <QRegularExpression>

#include <cctype>

// ---- 转义展开 ----

QByteArray DKeyEntry::unescape(const QByteArray &input)
{
    QByteArray result(input);

    for (int i = 0; i < result.count() - 1; ++i) {
        const char ch = result[i];
        if (ch != '\\')
            continue;

        char replacement[2] = {0, 0};
        int charsToRemove = 2;
        bool escapedChar = true;

        switch (result[i + 1]) {
        case 'E':
            replacement[0] = 27;
            break;
        case 'b':
            replacement[0] = 8;
            break;
        case 'f':
            replacement[0] = 12;
            break;
        case 't':
            replacement[0] = 9;
            break;
        case 'r':
            replacement[0] = 13;
            break;
        case 'n':
            replacement[0] = 10;
            break;
        case 'x': {
            // \xh 或 \xhh：后随一到两个十六进制位
            char hexDigits[3] = {0};
            if (i < result.count() - 2 && std::isxdigit(static_cast<uchar>(result[i + 2])))
                hexDigits[0] = result[i + 2];
            if (i < result.count() - 3 && std::isxdigit(static_cast<uchar>(result[i + 3])))
                hexDigits[1] = result[i + 3];

            unsigned charValue = 0;
            std::sscanf(hexDigits, "%x", &charValue);
            replacement[0] = static_cast<char>(charValue);
            charsToRemove = 2 + static_cast<int>(std::strlen(hexDigits));
            break;
        }
        default:
            escapedChar = false;
            break;
        }

        if (escapedChar)
            result.replace(i, charsToRemove, replacement, 1);
    }

    return result;
}

// ---- 匹配 ----

bool DKeyEntry::matches(int keyCode, Qt::KeyboardModifiers modifiers, quint16 testState) const
{
    if (m_keyCode != keyCode)
        return false;

    if ((modifiers & m_modifierMask) != (m_modifiers & m_modifierMask))
        return false;

    // 修饰键非零时隐式满足 AnyModifier 状态
    if ((modifiers & ~Qt::KeypadModifier) != 0)
        testState |= KeyStateAnyModifier;

    if ((testState & m_stateMask) != (m_state & m_stateMask))
        return false;

    // AnyModifier 特殊处理：小键盘修饰不计数
    const bool anyModifiersSet = modifiers != 0 && modifiers != Qt::KeypadModifier;
    const bool wantAnyModifier = m_state & KeyStateAnyModifier;
    if ((m_stateMask & KeyStateAnyModifier) && (wantAnyModifier != anyModifiersSet))
        return false;

    return true;
}

QByteArray DKeyEntry::text(bool expandWildcards, Qt::KeyboardModifiers modifiers) const
{
    QByteArray expandedText = m_text;

    if (expandWildcards) {
        // xterm 修饰参数：1 无修饰，Shift +1，Alt +2，Ctrl +4
        int modifierValue = 1;
        modifierValue += (modifiers & Qt::ShiftModifier) ? 1 : 0;
        modifierValue += ((modifiers & Qt::AltModifier) ? 1 : 0) << 1;
        modifierValue += ((modifiers & Qt::ControlModifier) ? 1 : 0) << 2;

        for (char &c : expandedText) {
            if (c == '*')
                c = '0' + modifierValue;
        }
    }

    return expandedText;
}

// ---- 键盘映射表 ----

DKeyboardTranslator::DKeyboardTranslator(const QString &name)
    : m_name(name)
{
}

DKeyEntry DKeyboardTranslator::findEntry(int keyCode, Qt::KeyboardModifiers modifiers, quint16 state) const
{
    for (const DKeyEntry &entry : m_entries) {
        if (entry.keyCode() == keyCode && entry.matches(keyCode, modifiers, state))
            return entry;
    }
    return DKeyEntry();
}

void DKeyboardTranslator::replaceEntry(const DKeyEntry &oldEntry, const DKeyEntry &newEntry)
{
    const int index = m_entries.indexOf(oldEntry);
    if (index != -1)
        m_entries.replace(index, newEntry);
}

namespace {

// 修饰键名解析
bool parseModifierName(const QString &item, Qt::KeyboardModifier &modifier)
{
    if (item == QLatin1String("shift"))
        modifier = Qt::ShiftModifier;
    else if (item == QLatin1String("ctrl") || item == QLatin1String("control"))
        modifier = Qt::ControlModifier;
    else if (item == QLatin1String("alt"))
        modifier = Qt::AltModifier;
    else if (item == QLatin1String("meta"))
        modifier = Qt::MetaModifier;
    else if (item == QLatin1String("keypad"))
        modifier = Qt::KeypadModifier;
    else
        return false;
    return true;
}

// 终端状态名解析
bool parseStateName(const QString &item, quint16 &flag)
{
    if (item == QLatin1String("appcukeys") || item == QLatin1String("appcursorkeys"))
        flag = KeyStateCursorKeys;
    else if (item == QLatin1String("ansi"))
        flag = KeyStateAnsi;
    else if (item == QLatin1String("newline"))
        flag = KeyStateNewLine;
    else if (item == QLatin1String("appscreen"))
        flag = KeyStateAlternate;
    else if (item == QLatin1String("anymod") || item == QLatin1String("anymodifier"))
        flag = KeyStateAnyModifier;
    else if (item == QLatin1String("appkeypad"))
        flag = KeyStateAppKeypad;
    else
        return false;
    return true;
}

// 按键名解析（QKeySequence 名称 + 历史兼容别名）
bool parseKeyName(const QString &item, int &keyCode)
{
    const QKeySequence sequence = QKeySequence::fromString(item);
    if (!sequence.isEmpty()) {
        keyCode = sequence[0];
        if (sequence.count() > 1)
            qDebug() << "Unhandled key codes in sequence:" << item;
    } else if (item == QLatin1String("prior")) {
        keyCode = Qt::Key_PageUp;
    } else if (item == QLatin1String("next")) {
        keyCode = Qt::Key_PageDown;
    } else {
        return false;
    }
    return true;
}

// 命令名解析
bool parseCommandName(const QString &text, quint8 &command)
{
    if (text.compare(QLatin1String("erase"), Qt::CaseInsensitive) == 0)
        command = KeyCmdErase;
    else if (text.compare(QLatin1String("scrollpageup"), Qt::CaseInsensitive) == 0)
        command = KeyCmdScrollPageUp;
    else if (text.compare(QLatin1String("scrollpagedown"), Qt::CaseInsensitive) == 0)
        command = KeyCmdScrollPageDown;
    else if (text.compare(QLatin1String("scrolllineup"), Qt::CaseInsensitive) == 0)
        command = KeyCmdScrollLineUp;
    else if (text.compare(QLatin1String("scrolllinedown"), Qt::CaseInsensitive) == 0)
        command = KeyCmdScrollLineDown;
    else if (text.compare(QLatin1String("scrolllock"), Qt::CaseInsensitive) == 0)
        command = KeyCmdScrollLock;
    else if (text.compare(QLatin1String("scrolluptotop"), Qt::CaseInsensitive) == 0)
        command = KeyCmdScrollTop;
    else if (text.compare(QLatin1String("scrolldowntobottom"), Qt::CaseInsensitive) == 0)
        command = KeyCmdScrollBottom;
    else
        return false;
    return true;
}

// 条件串解析："+Name" 期望成立、"−Name" 期望不成立
void decodeCondition(const QString &text, int &keyCode,
                     Qt::KeyboardModifiers &modifiers, Qt::KeyboardModifiers &modifierMask,
                     quint16 &flags, quint16 &flagMask)
{
    bool wanted = true;
    QString buffer;

    for (int i = 0; i < text.count(); ++i) {
        const QChar &ch = text[i];
        const bool isFirstLetter = (i == 0);
        const bool isLastLetter = (i == text.count() - 1);
        bool endOfItem = true;

        if (ch.isLetterOrNumber()) {
            endOfItem = false;
            buffer.append(ch);
        } else if (isFirstLetter) {
            buffer.append(ch);
        }

        if ((endOfItem || isLastLetter) && !buffer.isEmpty()) {
            Qt::KeyboardModifier itemModifier = Qt::NoModifier;
            quint16 itemFlag = KeyStateNone;
            int itemKeyCode = 0;

            if (parseModifierName(buffer, itemModifier)) {
                modifierMask |= itemModifier;
                if (wanted)
                    modifiers |= itemModifier;
            } else if (parseStateName(buffer, itemFlag)) {
                flagMask |= itemFlag;
                if (wanted)
                    flags |= itemFlag;
            } else if (parseKeyName(buffer, itemKeyCode)) {
                keyCode = itemKeyCode;
            } else {
                qDebug() << "Unable to parse key binding item:" << buffer;
            }

            buffer.clear();
        }

        if (ch == QLatin1Char('+'))
            wanted = true;
        else if (ch == QLatin1Char('-'))
            wanted = false;
    }
}

}  // namespace

void DKeyboardTranslator::parseLine(const QString &line)
{
    QString text = line;

    // 去掉引号外注释
    bool inQuotes = false;
    int commentPos = -1;
    for (int i = text.length() - 1; i >= 0; --i) {
        const QChar ch = text[i];
        if (ch == QLatin1Char('"'))
            inQuotes = !inQuotes;
        else if (ch == QLatin1Char('#') && !inQuotes)
            commentPos = i;
    }
    if (commentPos != -1)
        text.remove(commentPos, text.length() - commentPos);

    text = text.simplified();
    if (text.isEmpty())
        return;

    // 标题行：keyboard "描述"
    static const QRegularExpression titlePattern(QLatin1String("keyboard\\s+\"(.*)\""));
    // 键位行：key 条件 : "输出" | 命令
    static const QRegularExpression keyPattern(QLatin1String("key\\s+([\\w\\+\\s\\-\\*\\.]+)\\s*:\\s*(\"(.*)\"|\\w+)"));

    const QRegularExpressionMatch titleMatch = titlePattern.match(text);
    if (titleMatch.hasMatch()) {
        m_description = titleMatch.captured(1);
        return;
    }

    const QRegularExpressionMatch keyMatch = keyPattern.match(text);
    if (!keyMatch.hasMatch())
        return;

    const QString condition = keyMatch.captured(1).toLower();
    const QString rawResult = keyMatch.captured(3);

    int keyCode = Qt::Key_unknown;
    Qt::KeyboardModifiers modifiers = Qt::NoModifier;
    Qt::KeyboardModifiers modifierMask = Qt::NoModifier;
    quint16 flags = KeyStateNone;
    quint16 flagMask = KeyStateNone;
    decodeCondition(condition, keyCode, modifiers, modifierMask, flags, flagMask);

    quint8 command = KeyCmdNone;
    QByteArray entryText;
    if (!rawResult.isEmpty()) {
        entryText = rawResult.toLocal8Bit();
    } else {
        const QString commandName = keyMatch.captured(2);
        if (!parseCommandName(commandName, command))
            qDebug() << "Command" << commandName << "not understood.";
    }

    DKeyEntry entry;
    entry.setKeyCode(keyCode);
    entry.setState(flags);
    entry.setStateMask(flagMask);
    entry.setModifiers(modifiers);
    entry.setModifierMask(modifierMask);
    entry.setText(entryText);
    entry.setCommand(command);

    m_entries.append(entry);
}

DKeyboardTranslator *DKeyboardTranslator::loadFromFile(const QString &path, const QString &name)
{
    QFile source(path);
    if (!source.open(QIODevice::ReadOnly | QIODevice::Text))
        return nullptr;

    const QString translatorName = name.isEmpty() ? QFileInfo(path).baseName() : name;
    auto *translator = new DKeyboardTranslator(translatorName);

    while (!source.atEnd())
        translator->parseLine(QString::fromUtf8(source.readLine()));

    source.close();
    return translator;
}
