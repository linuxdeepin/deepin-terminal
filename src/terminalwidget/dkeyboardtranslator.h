// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DKEYBOARDTRANSLATOR_H
#define DKEYBOARDTRANSLATOR_H

#include <QByteArray>
#include <QList>
#include <QString>
#include <Qt>

// 键位匹配时的终端状态位
enum DKeyState : quint16 {
    KeyStateNone = 0,
    KeyStateNewLine = 1,
    KeyStateAnsi = 2,
    KeyStateCursorKeys = 4,
    KeyStateAlternate = 8,
    KeyStateAnyModifier = 16,
    KeyStateAppKeypad = 32
};

// 键位对应的命令
enum DKeyCommand : quint16 {
    KeyCmdNone = 0,
    KeyCmdSend = 1,
    KeyCmdScrollPageUp = 2,
    KeyCmdScrollPageDown = 4,
    KeyCmdScrollLineUp = 8,
    KeyCmdScrollLineDown = 16,
    KeyCmdScrollLock = 32,
    KeyCmdScrollTop = 64,
    KeyCmdScrollBottom = 128,
    KeyCmdErase = 256
};

/*******************************************************************************
 1. @类名:    DKeyEntry
 2. @说明:    键位绑定条目：按键 + 修饰键条件 + 终端状态条件 → 输出字节串/命令。
            文本中的转义（\E \n \t \xHH 等）在解析时展开；
            输出文本中的 '*' 通配符按修饰键组合展开为 xterm 修饰参数。
*******************************************************************************/
class DKeyEntry
{
public:
    DKeyEntry() = default;

    bool isNull() const
    {
        return m_keyCode == 0 && m_text.isEmpty() && m_command == KeyCmdNone;
    }

    int keyCode() const { return m_keyCode; }
    void setKeyCode(int keyCode) { m_keyCode = keyCode; }

    Qt::KeyboardModifiers modifiers() const { return m_modifiers; }
    void setModifiers(Qt::KeyboardModifiers modifiers) { m_modifiers = modifiers; }

    Qt::KeyboardModifiers modifierMask() const { return m_modifierMask; }
    void setModifierMask(Qt::KeyboardModifiers mask) { m_modifierMask = mask; }

    quint16 state() const { return m_state; }
    void setState(quint16 state) { m_state = state; }

    quint16 stateMask() const { return m_stateMask; }
    void setStateMask(quint16 mask) { m_stateMask = mask; }

    quint16 command() const { return m_command; }
    void setCommand(quint16 command) { m_command = command; }

    /** 输出文本；expandWildcards 时把 '*' 展开为修饰键参数（1-8） */
    QByteArray text(bool expandWildcards = false,
                    Qt::KeyboardModifiers modifiers = Qt::NoModifier) const;
    /** 设置输出文本（转义在此处展开） */
    void setText(const QByteArray &text) { m_text = unescape(text); }

    bool matches(int keyCode, Qt::KeyboardModifiers modifiers, quint16 testState) const;

    bool operator==(const DKeyEntry &other) const
    {
        return m_keyCode == other.m_keyCode
                && m_modifiers == other.m_modifiers
                && m_modifierMask == other.m_modifierMask
                && m_state == other.m_state
                && m_stateMask == other.m_stateMask
                && m_command == other.m_command
                && m_text == other.m_text;
    }

private:
    static QByteArray unescape(const QByteArray &input);

    int m_keyCode = 0;
    Qt::KeyboardModifiers m_modifiers = Qt::NoModifier;
    Qt::KeyboardModifiers m_modifierMask = Qt::NoModifier;
    quint16 m_state = KeyStateNone;
    quint16 m_stateMask = KeyStateNone;
    quint16 m_command = KeyCmdNone;
    QByteArray m_text;
};

/*******************************************************************************
 1. @类名:    DKeyboardTranslator
 2. @说明:    键盘映射表。解析 keytab 数据文件，
            按键 + 修饰键 + 终端状态 → 输出字节串/命令。
            解析器直接内置。
*******************************************************************************/
class DKeyboardTranslator
{
public:
    explicit DKeyboardTranslator(const QString &name);

    QString name() const { return m_name; }
    void setName(const QString &name) { m_name = name; }

    QString description() const { return m_description; }
    void setDescription(const QString &description) { m_description = description; }

    DKeyEntry findEntry(int keyCode, Qt::KeyboardModifiers modifiers, quint16 state = KeyStateNone) const;
    void addEntry(const DKeyEntry &entry) { m_entries.append(entry); }
    void replaceEntry(const DKeyEntry &oldEntry, const DKeyEntry &newEntry);
    int entryCount() const { return m_entries.count(); }

    /** 从 keytab 文件加载（失败返回 nullptr）；name 缺省取文件名 */
    static DKeyboardTranslator *loadFromFile(const QString &path, const QString &name = QString());

private:
    void parseLine(const QString &line);

    QString m_name;
    QString m_description;
    QList<DKeyEntry> m_entries;
};

#endif  // DKEYBOARDTRANSLATOR_H
