// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_dbusmanager_test.h"
#include "dbusmanager.h"
#include "ut_stub_defines.h"

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
//TEST_F(UT_Dbusmanager_Test, standardfont)
//{
//    UT_STUB_QDBUS_CALL_CREATE
//    m_pDbusManager->callAppearanceFont("standardfont");
//    //dbus call被调用过
//    EXPECT_TRUE(UT_STUB_QDBUS_CALL_RESULT);


//}

//获取字体 等宽字体
//TEST_F(UT_Dbusmanager_Test, monospacefont)
//{
//    UT_STUB_QDBUS_CALL_CREATE
//    m_pDbusManager->callAppearanceFont("monospacefont");
//    //dbus call被调用过
//    EXPECT_TRUE(UT_STUB_QDBUS_CALL_RESULT);

//}

//获取字体,其他错误字体
//TEST_F(UT_Dbusmanager_Test, otherfailfont)
//{
//    UT_STUB_QDBUS_CALL_CREATE
//    m_pDbusManager->callAppearanceFont("otherfailfont");
//    //dbus call被调用过
//    EXPECT_TRUE(UT_STUB_QDBUS_CALL_RESULT);

//}

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

#endif

