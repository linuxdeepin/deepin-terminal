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

#include <QDebug>

UT_Dbusmanager_Test::UT_Dbusmanager_Test()
{
    m_pDbusManager = new DBusManager();
    m_pDbusManager->initDBus();
}

UT_Dbusmanager_Test::~UT_Dbusmanager_Test()
{
    delete  m_pDbusManager;
}

#ifdef UT_DBUSMANAGER_TEST

//设置桌面索引
TEST_F(UT_Dbusmanager_Test, callKDECurrentDesktopIndex)
{
    m_pDbusManager->callKDESetCurrentDesktop(1);
}

//获取字体 标准字体
TEST_F(UT_Dbusmanager_Test, standardfont)
{
    m_pDbusManager->callAppearanceFont("standardfont");
}

//获取字体 等宽字体
TEST_F(UT_Dbusmanager_Test, monospacefont)
{
    m_pDbusManager->callAppearanceFont("monospacefont");
}

//获取字体,其他错误字体
TEST_F(UT_Dbusmanager_Test, otherfailfont)
{
    QStringList lstFont = m_pDbusManager->callAppearanceFont("otherfailfont");
    EXPECT_EQ(lstFont.size(), 0);
}

//唤醒终端
TEST_F(UT_Dbusmanager_Test, callTerminal)
{
    m_pDbusManager->callTerminalEntry({"deepin-terminal", "-C", "void_script"});
}


//Json转为 QStringList
TEST_F(UT_Dbusmanager_Test, converToList)
{
    QJsonArray array = { 1, 2.2, QString() };
    DBusManager::converToList("type", array);
}

TEST_F(UT_Dbusmanager_Test, listenTouchPadSignal)
{
    m_pDbusManager->listenTouchPadSignal();
}

TEST_F(UT_Dbusmanager_Test, listenDesktopSwitched)
{
    m_pDbusManager->listenDesktopSwitched();
}

#endif

