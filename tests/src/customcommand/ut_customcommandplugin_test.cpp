// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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


