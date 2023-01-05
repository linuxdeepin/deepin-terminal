// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_serverconfiggrouppanel_test.h"
#include "serverconfiggrouppanel.h"
#include "service.h"
#include "mainwindow.h"
#include "ut_stub_defines.h"

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>

//Google GTest 相关头文件
#include <gtest/gtest.h>

UT_ServerConfigGroupPanel_Test::UT_ServerConfigGroupPanel_Test()
{
}

void UT_ServerConfigGroupPanel_Test::SetUp()
{
    if (!Service::instance()->property("isServiceInit").toBool()) {
        Service::instance()->setProperty("isServiceInit", true);
    }

    m_normalTermProperty[QuakeMode] = false;
    m_normalTermProperty[SingleFlag] = true;
    m_normalWindow = new NormalWindow(m_normalTermProperty, nullptr);
    m_normalWindow->resize(800, 600);
    m_normalWindow->show();
}

void UT_ServerConfigGroupPanel_Test::TearDown()
{
    delete m_normalWindow;
}

#ifdef UT_SERVERCONFIGGROUPPANEL_TEST
TEST_F(UT_ServerConfigGroupPanel_Test, refreshData)
{
    EXPECT_EQ(m_normalWindow->isVisible(), true);

    m_normalWindow->showPlugin(MainWindow::PLUGIN_TYPE_REMOTEMANAGEMENT);

    RemoteManagementPlugin *remotePlugin = m_normalWindow->findChild<RemoteManagementPlugin *>();
    RemoteManagementTopPanel *remoteTopPanel = remotePlugin->getRemoteManagementTopPanel();
    EXPECT_NE(remoteTopPanel, nullptr);
    EXPECT_EQ(remoteTopPanel->isVisible(), true);

    ServerConfigGroupPanel *groupPanel = remoteTopPanel->findChild<ServerConfigGroupPanel *>();
    EXPECT_NE(groupPanel, nullptr);
    groupPanel->refreshData("group01");
}

TEST_F(UT_ServerConfigGroupPanel_Test, setFocusBack)
{
    EXPECT_EQ(m_normalWindow->isVisible(), true);

    m_normalWindow->showPlugin(MainWindow::PLUGIN_TYPE_REMOTEMANAGEMENT);

    RemoteManagementPlugin *remotePlugin = m_normalWindow->findChild<RemoteManagementPlugin *>();
    RemoteManagementTopPanel *remoteTopPanel = remotePlugin->getRemoteManagementTopPanel();
    EXPECT_NE(remoteTopPanel, nullptr);
    EXPECT_EQ(remoteTopPanel->isVisible(), true);

    ServerConfigGroupPanel *groupPanel = remoteTopPanel->findChild<ServerConfigGroupPanel *>();
    EXPECT_NE(groupPanel, nullptr);
    groupPanel->setFocusBack();
}

TEST_F(UT_ServerConfigGroupPanel_Test, clearAllFocus)
{
    EXPECT_EQ(m_normalWindow->isVisible(), true);

    m_normalWindow->showPlugin(MainWindow::PLUGIN_TYPE_REMOTEMANAGEMENT);

    RemoteManagementPlugin *remotePlugin = m_normalWindow->findChild<RemoteManagementPlugin *>();
    RemoteManagementTopPanel *remoteTopPanel = remotePlugin->getRemoteManagementTopPanel();
    EXPECT_NE(remoteTopPanel, nullptr);
    EXPECT_EQ(remoteTopPanel->isVisible(), true);

    ServerConfigGroupPanel *groupPanel = remoteTopPanel->findChild<ServerConfigGroupPanel *>();
    EXPECT_NE(groupPanel, nullptr);
    groupPanel->clearAllFocus();

    EXPECT_EQ(groupPanel->m_rebackButton->hasFocus(), false);
    EXPECT_EQ(groupPanel->m_searchEdit->hasFocus(), false);
    EXPECT_EQ(groupPanel->m_listWidget->hasFocus(), false);
}

/*******************************************************************************
 1. @函数:    refreshDataTest
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-24
 4. @说明:    刷新列表 => 组内数据
*******************************************************************************/
TEST_F(UT_ServerConfigGroupPanel_Test, refreshDataTest)
{
    // 创建分组
    ServerConfig groupConfig;
    groupConfig.m_serverName = "group_item";
    groupConfig.m_address = "127.0.0.1";
    groupConfig.m_group = "group2020";
    groupConfig.m_userName = "dzw";
    groupConfig.m_port = "22";
    // 分组数据存储结构
    QList<ServerConfig *> list;
    list.append(&groupConfig);
    // 数据存储结构
    // 清空数据
    ServerConfigManager::instance()->m_serverConfigs.clear();
    // 装填数据
    ServerConfigManager::instance()->m_serverConfigs.insert("group2020", list);

    // 新建界面
    ServerConfigGroupPanel groupPanel;
    // 刷新界面
    groupPanel.refreshData("group2020");
    // 界面下只有一个数据
    int count = groupPanel.m_listWidget->count();
    EXPECT_EQ(count, 1);

    // 不存在的组
    groupPanel.refreshData("group1988");
    count = groupPanel.m_listWidget->count();
    EXPECT_EQ(count, 0);

    // 清空数据
    ServerConfigManager::instance()->m_serverConfigs.clear();
}

/*******************************************************************************
 1. @函数:    setFocusBackTest
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-24
 4. @说明:    测试界面返回
*******************************************************************************/
TEST_F(UT_ServerConfigGroupPanel_Test, setFocusBackTest)
{
    // 创建分组
    ServerConfig groupConfig;
    groupConfig.m_serverName = "group_item";
    groupConfig.m_address = "127.0.0.1";
    groupConfig.m_group = "group2020";
    groupConfig.m_userName = "dzw";
    groupConfig.m_port = "22";
    ServerConfig groupConfig2;
    groupConfig2.m_serverName = "group_item2";
    groupConfig2.m_address = "127.0.0.1";
    groupConfig2.m_group = "group2020";
    groupConfig2.m_userName = "dzw";
    groupConfig2.m_port = "22";
    // 分组数据存储结构
    QList<ServerConfig *> list;
    list.append(&groupConfig);
    list.append(&groupConfig);
    // 数据存储结构
    // 清空数据
    ServerConfigManager::instance()->m_serverConfigs.clear();
    // 装填数据
    ServerConfigManager::instance()->m_serverConfigs.insert("group2020", list);
    //添加group2020
    EXPECT_TRUE(ServerConfigManager::instance()->m_serverConfigs.contains("group2020"));

    // 新建界面
    ServerConfigGroupPanel groupPanel;
    // 刷新界面
    groupPanel.refreshData("group2020");
    //当前group为group2020
    EXPECT_TRUE("group2020" == groupPanel.m_groupName);
    // 刷新搜索框状态
    UT_STUB_QWIDGET_SETVISIBLE_CREATE;
    groupPanel.refreshSearchState();
    //会触发setvisible函数
    EXPECT_TRUE(UT_STUB_QWIDGET_SETVISIBLE_RESULT);

    // 两个数据 => 有搜索框
    UT_STUB_QWIDGET_ISVISIBLE_APPEND;
    UT_STUB_QWIDGET_SETFOCUS_APPEND;
    groupPanel.setFocusBack();
    //会触发isvisible喊setFocus函数
    EXPECT_TRUE(UT_STUB_QWIDGET_ISVISIBLE_RESULT);
    EXPECT_TRUE(UT_STUB_QWIDGET_SETFOCUS_RESULT);

    // 影藏搜索框
    groupPanel.m_searchEdit->hide();
    groupPanel.setFocusBack();

    // 清空数据
    ServerConfigManager::instance()->m_serverConfigs.clear();
    // 刷新界面
    groupPanel.refreshData("group2020");
    groupPanel.setFocusBack();
}

/*******************************************************************************
 1. @函数:    clearAllFocusTest
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-24
 4. @说明:    测试清除焦点是否成功
*******************************************************************************/
TEST_F(UT_ServerConfigGroupPanel_Test, clearAllFocusTest)
{
    ServerConfigGroupPanel groupPanel;
    groupPanel.show();
    groupPanel.refreshData("group2020");
    groupPanel.clearAllFocus();
    EXPECT_EQ(groupPanel.m_rebackButton->hasFocus(), false);
    EXPECT_EQ(groupPanel.m_listWidget->hasFocus(), false);
    EXPECT_EQ(groupPanel.m_searchEdit->hasFocus(), false);
}

/*******************************************************************************
 1. @函数:    lambda
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-24
 4. @说明:    测试lambda表达式,是否会影响进程
*******************************************************************************/
TEST_F(UT_ServerConfigGroupPanel_Test, lambda)
{
    // 初始化界面
    ServerConfigGroupPanel groupPanel;
    groupPanel.show();
    groupPanel.refreshData("group2020");
    groupPanel.m_isShow = true;

    groupPanel.onListViewFocusOut(Qt::OtherFocusReason);
    //会选择第一行
    EXPECT_TRUE(groupPanel.m_listWidget->currentIndex() == -1);

    UT_STUB_QWIDGET_ISVISIBLE_CREATE;
    groupPanel.onListViewFocusOut(Qt::BacktabFocusReason);
    //会执行isVisible()函数
    EXPECT_TRUE(UT_STUB_QWIDGET_ISVISIBLE_RESULT);

    //没有焦点
    groupPanel.onListViewFocusOut(Qt::NoFocusReason);
    EXPECT_TRUE(groupPanel.m_listWidget->currentIndex() == -1);


    emit ServerConfigManager::instance()->refreshList();


}
#endif
