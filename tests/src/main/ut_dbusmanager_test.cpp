// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_dbusmanager_test.h"
#include "dbusmanager.h"
#include "settings.h"
#include "ut_stub_defines.h"

#include <QSignalSpy>

UT_Dbusmanager_Test::UT_Dbusmanager_Test()
{
    m_pDbusManager = new DBusManager();
    m_pDbusManager->initDBus();
}

UT_Dbusmanager_Test::~UT_Dbusmanager_Test()
{
    m_pDbusManager->deleteLater();
}

#ifdef UT_DBUSMANAGER_TEST

//设置桌面索引
TEST_F(UT_Dbusmanager_Test, callKDECurrentDesktopIndex)
{
    UT_STUB_QDBUS_CALL_CREATE
    m_pDbusManager->callKDESetCurrentDesktop(1);
    //dbus call被调用过
    EXPECT_TRUE(UT_STUB_QDBUS_CALL_RESULT);
}

//获取字体 标准字体
TEST_F(UT_Dbusmanager_Test, standardfont)
{
    UT_STUB_QDBUS_CALL_CREATE
    m_pDbusManager->callAppearanceFont("standardfont");
    //dbus call被调用过
    EXPECT_TRUE(UT_STUB_QDBUS_CALL_RESULT);


}

//获取字体 等宽字体
TEST_F(UT_Dbusmanager_Test, monospacefont)
{
    UT_STUB_QDBUS_CALL_CREATE
    m_pDbusManager->callAppearanceFont("monospacefont");
    //dbus call被调用过
    EXPECT_TRUE(UT_STUB_QDBUS_CALL_RESULT);

}

//获取字体,其他错误字体
TEST_F(UT_Dbusmanager_Test, otherfailfont)
{
    UT_STUB_QDBUS_CALL_CREATE
    m_pDbusManager->callAppearanceFont("otherfailfont");
    //dbus call被调用过
    EXPECT_TRUE(UT_STUB_QDBUS_CALL_RESULT);

}

//唤醒终端
TEST_F(UT_Dbusmanager_Test, callTerminal)
{
    UT_STUB_QDBUS_CALL_CREATE
    m_pDbusManager->callTerminalEntry({"deepin-terminal", "-C", "void_script"});
    //dbus call被调用过
    EXPECT_TRUE(UT_STUB_QDBUS_CALL_RESULT);;
}

TEST_F(UT_Dbusmanager_Test, listenTouchPadSignal)
{
    UT_STUB_QDBUS_CONNECT_CREATE
    m_pDbusManager->listenTouchPadSignal();
    //dbus connect被调用过
    EXPECT_TRUE(UT_STUB_QDBUS_CONNECT_RESULT);
}

TEST_F(UT_Dbusmanager_Test, listenDesktopSwitched)
{
    UT_STUB_QDBUS_CONNECT_CREATE
    m_pDbusManager->listenDesktopSwitched();
    //dbus connect被调用过
    EXPECT_TRUE(UT_STUB_QDBUS_CONNECT_RESULT);
}

// 测试 entry 函数，验证信号 entryArgs 会被发射
TEST_F(UT_Dbusmanager_Test, entry)
{
    QStringList args = {"deepin-terminal", "-e", "ls"};
    QSignalSpy spy(m_pDbusManager, SIGNAL(entryArgs(QStringList)));
    m_pDbusManager->entry(args);
    EXPECT_EQ(spy.count(), 1);
}

// 测试 callKDECurrentDesktop 函数（DBus call 通过 stub 拦截，返回 InvalidMessage，函数返回 -1）
TEST_F(UT_Dbusmanager_Test, callKDECurrentDesktop)
{
    UT_STUB_QDBUS_CALL_CREATE
    int result = m_pDbusManager->callKDECurrentDesktop();
    // stub 返回空 QDBusMessage，type() != ReplyMessage，应返回 -1
    EXPECT_EQ(result, -1);
    EXPECT_TRUE(UT_STUB_QDBUS_CALL_RESULT);
}

// 字体大小 getter/setter
TEST_F(UT_Dbusmanager_Test, consoleFontSize)
{
    const int expectSize = 18;
    Settings::instance()->setFontSize(expectSize);
    EXPECT_EQ(m_pDbusManager->consoleFontSize(), expectSize);
}

TEST_F(UT_Dbusmanager_Test, setConsoleFontSize)
{
    const int newSize = 22;
    m_pDbusManager->setConsoleFontSize(newSize);
    EXPECT_EQ(Settings::instance()->fontSize(), newSize);
}

// 字体 family getter/setter
TEST_F(UT_Dbusmanager_Test, consoleFontFamily)
{
    // 验证 getter 可正常返回（setFontName 依赖 DBus 字体列表，测试环境下走默认值）
    const QString family = m_pDbusManager->consoleFontFamily();
    EXPECT_FALSE(family.isNull());
}

TEST_F(UT_Dbusmanager_Test, setConsoleFontFamily)
{
    // setFontName 会查询 DBus 字体列表，测试环境下大概率匹配不上，仅验证不崩溃
    m_pDbusManager->setConsoleFontFamily(QStringLiteral("Noto Sans Mono"));
    SUCCEED();
}

// 透明度 getter/setter（Settings 内部把 int 百分比换算为 0~1 的 qreal）
TEST_F(UT_Dbusmanager_Test, consoleOpacity)
{
    const int expectPercent = 85;
    Settings::instance()->setOpacity(expectPercent);
    EXPECT_NEAR(m_pDbusManager->consoleOpacity(), expectPercent / 100.0, 0.001);
}

TEST_F(UT_Dbusmanager_Test, setConsoleOpacity)
{
    const int newOpacity = 60;
    m_pDbusManager->setConsoleOpacity(newOpacity);
    EXPECT_NEAR(Settings::instance()->opacity(), newOpacity / 100.0, 0.001);
}

// 光标形状 getter/setter
TEST_F(UT_Dbusmanager_Test, consoleCursorShape)
{
    const int expectShape = 1;
    Settings::instance()->setCursorShape(expectShape);
    EXPECT_EQ(m_pDbusManager->consoleCursorShape(), expectShape);
}

TEST_F(UT_Dbusmanager_Test, setConsoleCursorShape)
{
    const int newShape = 2;
    m_pDbusManager->setConsoleCursorShape(newShape);
    EXPECT_EQ(Settings::instance()->cursorShape(), newShape);
}

// 光标闪烁 getter/setter
TEST_F(UT_Dbusmanager_Test, consoleCursorBlink)
{
    const bool expectBlink = true;
    Settings::instance()->setCursorBlink(expectBlink);
    EXPECT_EQ(m_pDbusManager->consoleCursorBlink(), expectBlink);
}

TEST_F(UT_Dbusmanager_Test, setConsoleCursorBlink)
{
    const bool newBlink = false;
    m_pDbusManager->setConsoleCursorBlink(newBlink);
    EXPECT_EQ(Settings::instance()->cursorBlink(), newBlink);
}

// 配色方案 getter/setter
TEST_F(UT_Dbusmanager_Test, consoleColorScheme)
{
    EXPECT_FALSE(m_pDbusManager->consoleColorScheme().isEmpty());
}

TEST_F(UT_Dbusmanager_Test, setConsoleColorScheme)
{
    const QString newScheme = QStringLiteral("Dark");
    m_pDbusManager->setConsoleColorScheme(newScheme);
    EXPECT_EQ(Settings::instance()->colorScheme(), newScheme);
}

// shell 路径 getter/setter
TEST_F(UT_Dbusmanager_Test, consoleShell)
{
    EXPECT_FALSE(m_pDbusManager->consoleShell().isEmpty());
}

TEST_F(UT_Dbusmanager_Test, setConsoleShell)
{
    // setConsoleShell 会从 /etc/shells 找匹配，匹配不上时不动设置；这里只验证不崩溃
    m_pDbusManager->setConsoleShell(QStringLiteral("/bin/bash"));
    SUCCEED();
}

// 测试 callSystemSound（依赖 DBus call，验证函数能正常执行）
TEST_F(UT_Dbusmanager_Test, callSystemSound)
{
    UT_STUB_QDBUS_CALL_CREATE
    m_pDbusManager->callSystemSound(QStringLiteral("message"));
    EXPECT_TRUE(UT_STUB_QDBUS_CALL_RESULT);
}

// 测试 callAppearanceFont 的重载版本（传入空列表）
TEST_F(UT_Dbusmanager_Test, callAppearanceFontFromList)
{
    UT_STUB_QDBUS_CALL_CREATE
    FontDataList result = m_pDbusManager->callAppearanceFont(QStringList(), QStringLiteral("monospacefont"));
    EXPECT_TRUE(result.isEmpty());
    EXPECT_TRUE(UT_STUB_QDBUS_CALL_RESULT);
}

#endif

