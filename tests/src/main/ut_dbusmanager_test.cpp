/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:     daizhengwen <daizhengwen@uniontech.com>
 *
 * Maintainer: daizhengwen <daizhengwen@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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

#endif

