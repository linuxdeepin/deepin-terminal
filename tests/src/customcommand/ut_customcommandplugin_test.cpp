/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:     wangliang <wangliang@uniontech.com>
 *
 * Maintainer: wangliang <wangliang@uniontech.com>
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

#include "ut_customcommandplugin_test.h"
#include "customcommandplugin.h"
#include "ut_stub_defines.h"
#include "mainwindow.h"
#include "termproperties.h"
#include "service.h"

//qt
#include <QTest>
#include <QtGui>
#include <QDebug>
#include <QSignalSpy>

UT_Customcommandplugin_Test::UT_Customcommandplugin_Test()
{

}

void UT_Customcommandplugin_Test::SetUp()
{
    if (!Service::instance()->property("isServiceInit").toBool())
    {
        Service::instance()->setProperty("isServiceInit", true);
    }
    m_cmdPlugin = new CustomCommandPlugin;
}

void UT_Customcommandplugin_Test::TearDown()
{
    DELETE_PTR_LATER(m_cmdPlugin->m_mainWindow);
    DELETE_PTR_LATER(m_cmdPlugin);
}

#ifdef UT_CUSTOMCOMMANDPLUGIN_TEST

TEST_F(UT_Customcommandplugin_Test,initPlugin)
{
    EXPECT_NE(m_cmdPlugin, nullptr);

    NormalWindow* normal = new NormalWindow(TermProperties("/"), nullptr);

    m_cmdPlugin->initPlugin(normal);
    EXPECT_NE(m_cmdPlugin->titlebarMenu(normal), nullptr);
    EXPECT_NE(m_cmdPlugin->getCustomCommandTopPanel(), nullptr);
}

TEST_F(UT_Customcommandplugin_Test,initPlugin001)
{
    EXPECT_NE(m_cmdPlugin, nullptr);

    QuakeWindow *quake = new QuakeWindow(TermProperties({{WorkingDir, "/"},{QuakeMode, true}}));
    m_cmdPlugin->initPlugin(quake);
    EXPECT_NE(m_cmdPlugin->titlebarMenu(quake), nullptr);
    EXPECT_NE(m_cmdPlugin->getCustomCommandTopPanel(), nullptr);
}

TEST_F(UT_Customcommandplugin_Test,doCustomCommand)
{
    CustomCommandPlugin *m_cmdPlugin = new CustomCommandPlugin;
    NormalWindow* normal = new NormalWindow(TermProperties("/"), nullptr);
    m_cmdPlugin->initPlugin(normal);

    m_cmdPlugin->m_isShow = true;
    m_cmdPlugin->doShowPlugin(MainWindow::PLUGIN_TYPE_NONE, false);
    //会更新m_cmdPlugin的m_isShow
    EXPECT_TRUE(!m_cmdPlugin->m_isShow);

    m_cmdPlugin->doCustomCommand("ls");
    //重置m_CurrentShowPlugin为PLUGIN_TYPE_NONE
    EXPECT_TRUE(m_cmdPlugin->m_mainWindow->m_CurrentShowPlugin == MainWindow::PLUGIN_TYPE_NONE);

    DELETE_PTR_LATER(m_cmdPlugin->m_mainWindow);
    DELETE_PTR_LATER(m_cmdPlugin);
}

#endif


