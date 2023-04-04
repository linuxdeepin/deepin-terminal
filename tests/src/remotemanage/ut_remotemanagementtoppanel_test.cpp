// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_remotemanagementtoppanel_test.h"
#include "remotemanagementtoppanel.h"
#include "service.h"
#include "mainwindow.h"
#include "../stub.h"
#include "ut_stub_defines.h"

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>

//Google GTest 相关头文件
#include <gtest/gtest.h>

void stub_focusCurrentPage_remote()
{
    return;
}

bool stub_isFocusOnList()
{
    return true;
}

UT_RemoteManagementTopPanel_Test::UT_RemoteManagementTopPanel_Test()
{
}

void UT_RemoteManagementTopPanel_Test::SetUp()
{
    if (!Service::instance()->property("isServiceInit").toBool()) {
        Service::instance()->setProperty("isServiceInit", true);
    }
}

void UT_RemoteManagementTopPanel_Test::TearDown()
{
}

static void doDeleteLater(RemoteManagementTopPanel *obj)
{
    obj->deleteLater();
}

#ifdef UT_REMOTEMANAGEMENTTOPPANEL_TEST

TEST_F(UT_RemoteManagementTopPanel_Test, setFocusInPanel)
{
    m_normalTermProperty[QuakeMode] = false;
    m_normalTermProperty[SingleFlag] = true;
    m_normalWindow = new NormalWindow(m_normalTermProperty, nullptr);
    m_normalWindow->resize(800, 600);
    m_normalWindow->show();
    EXPECT_EQ(m_normalWindow->isVisible(), true);

    m_normalWindow->showPlugin(MainWindow::PLUGIN_TYPE_REMOTEMANAGEMENT);

    RemoteManagementPlugin *remotePlugin = m_normalWindow->findChild<RemoteManagementPlugin *>();
    RemoteManagementTopPanel *remoteTopPanel = remotePlugin->getRemoteManagementTopPanel();
    EXPECT_NE(remoteTopPanel, nullptr);
    EXPECT_EQ(remoteTopPanel->isVisible(), true);

    remoteTopPanel->setFocusInPanel();
    m_normalWindow->close();
    delete m_normalWindow;
}

TEST_F(UT_RemoteManagementTopPanel_Test, showSearchPanel)
{
    m_normalTermProperty[QuakeMode] = false;
    m_normalTermProperty[SingleFlag] = true;
    m_normalWindow = new NormalWindow(m_normalTermProperty, nullptr);
    m_normalWindow->resize(800, 600);
    m_normalWindow->show();
    EXPECT_EQ(m_normalWindow->isVisible(), true);

    m_normalWindow->showPlugin(MainWindow::PLUGIN_TYPE_REMOTEMANAGEMENT);

    RemoteManagementPlugin *remotePlugin = m_normalWindow->findChild<RemoteManagementPlugin *>();
    RemoteManagementTopPanel *remoteTopPanel = remotePlugin->getRemoteManagementTopPanel();
    EXPECT_NE(remoteTopPanel, nullptr);
    EXPECT_EQ(remoteTopPanel->isVisible(), true);

    remoteTopPanel->showSearchPanel("group");
    m_normalWindow->close();
    delete m_normalWindow;
}

TEST_F(UT_RemoteManagementTopPanel_Test, showGroupPanel)
{
    m_normalTermProperty[QuakeMode] = false;
    m_normalTermProperty[SingleFlag] = true;
    m_normalWindow = new NormalWindow(m_normalTermProperty, nullptr);
    m_normalWindow->resize(800, 600);
    m_normalWindow->show();
    EXPECT_EQ(m_normalWindow->isVisible(), true);

    m_normalWindow->showPlugin(MainWindow::PLUGIN_TYPE_REMOTEMANAGEMENT);

    RemoteManagementPlugin *remotePlugin = m_normalWindow->findChild<RemoteManagementPlugin *>();
    RemoteManagementTopPanel *remoteTopPanel = remotePlugin->getRemoteManagementTopPanel();
    EXPECT_NE(remoteTopPanel, nullptr);
    EXPECT_EQ(remoteTopPanel->isVisible(), true);

    remoteTopPanel->showSearchPanel("group");

    //显示前一个界面（返回）
    remoteTopPanel->showPrevPanel();

    m_normalWindow->close();
    delete m_normalWindow;
}

/*******************************************************************************
 1. @函数:    ShowSearchPanel
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-23
 4. @说明:    显示搜索面板 => 组内搜索和无法搜索
*******************************************************************************/
TEST_F(UT_RemoteManagementTopPanel_Test, ShowSearchPanelTest)
{
    RemoteManagementTopPanel topPanel;
    // 没数据的情况
    // 显示分组界面的搜索
    topPanel.m_currentPanelType = ServerConfigManager::PanelType_Group;
    topPanel.m_group = "group";
    topPanel.showSearchPanel("1988");
    EXPECT_TRUE(topPanel.m_filterStack.contains("1988"));

    // 未知类型
    topPanel.m_currentPanelType = ServerConfigManager::PanelType_Search;
    topPanel.showSearchPanel("1995");
    EXPECT_TRUE(topPanel.m_filterStack.contains("1995"));
}

/*******************************************************************************
 1. @函数:    showGroupPanel
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-23
 4. @说明:    显示分组界面
*******************************************************************************/
TEST_F(UT_RemoteManagementTopPanel_Test, showGroupPanelTest)
{
    Stub s;
    s.set(ADDR(MainWindow, focusCurrentPage), stub_focusCurrentPage_remote);
    RemoteManagementTopPanel topPanel;
    // 没数据的情况
    // 不用界面显示分组界面
    topPanel.m_currentPanelType = ServerConfigManager::PanelType_Manage;
    topPanel.showGroupPanel("1988", false);
    EXPECT_TRUE("1988" == topPanel.m_group);
    EXPECT_TRUE("1988" == topPanel.m_serverConfigGroupPanel->m_groupName);
    //item 1988不获取焦点
    EXPECT_TRUE(false == topPanel.m_prevPanelStack.value(topPanel.m_prevPanelStack.size() - 1).m_isFocusOn);


    topPanel.m_currentPanelType = ServerConfigManager::PanelType_Manage;
    topPanel.showGroupPanel("1988", true);
    //item 1988获取焦点
    EXPECT_TRUE(true == topPanel.m_prevPanelStack.value(topPanel.m_prevPanelStack.size() - 1).m_isFocusOn);

    topPanel.m_currentPanelType = ServerConfigManager::PanelType_Search;
    topPanel.showGroupPanel("1995", true);
    EXPECT_TRUE("1995" == topPanel.m_group);
    EXPECT_TRUE("1995" == topPanel.m_serverConfigGroupPanel->m_groupName);
    //Search/item 1995获取焦点
    EXPECT_TRUE(true == topPanel.m_prevPanelStack.value(topPanel.m_prevPanelStack.size() - 1).m_isFocusOn);

    // 未知类型
    topPanel.m_currentPanelType = ServerConfigManager::PanelType_Group;
    topPanel.showGroupPanel("1995", true);
    EXPECT_TRUE("1995" == topPanel.m_group);
    EXPECT_TRUE("1995" == topPanel.m_serverConfigGroupPanel->m_groupName);
    //Group/item 1995获取焦点
    EXPECT_TRUE(true == topPanel.m_prevPanelStack.value(topPanel.m_prevPanelStack.size() - 1).m_isFocusOn);

}

/*******************************************************************************
 1. @函数:    showPrePanel
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-23
 4. @说明:    显示前一个界面
*******************************************************************************/
TEST_F(UT_RemoteManagementTopPanel_Test, showPrePanelTest)
{
    Stub s;
    s.set(ADDR(MainWindow, isFocusOnList), stub_isFocusOnList);
    // 清空堆栈
    QSharedPointer<RemoteManagementTopPanel> topPanel(new RemoteManagementTopPanel, doDeleteLater);
    topPanel->m_prevPanelStack.clear();
    // 显示前一个窗口
    UT_STUB_QWIDGET_SHOW_CREATE;
    topPanel->m_prevPanelStack << PanelState();
    topPanel->m_prevPanelStack.last().m_type = ServerConfigManager::PanelType_Manage;
    topPanel->showPrevPanel();
    EXPECT_TRUE(UT_STUB_QWIDGET_SHOW_RESULT);

    // 搜索返回
    UT_STUB_QWIDGET_SHOW_PREPARE;
    topPanel->m_prevPanelStack << PanelState();
    topPanel->m_prevPanelStack.last().m_type = ServerConfigManager::PanelType_Search;
    topPanel->m_filterStack.push_back(QString("hello"));
    topPanel->m_filterStack.push_back(QString("world"));
    topPanel->showPrevPanel();
    EXPECT_TRUE(UT_STUB_QWIDGET_SHOW_RESULT);

    // 分组返回
    UT_STUB_QWIDGET_SHOW_PREPARE;
    topPanel->m_prevPanelStack << PanelState();
    topPanel->m_prevPanelStack.last().m_type = ServerConfigManager::PanelType_Group;
    topPanel->showPrevPanel();
    EXPECT_TRUE(UT_STUB_QWIDGET_SHOW_RESULT);
}
#endif
