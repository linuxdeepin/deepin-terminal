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

#include "ut_remotemanagementtoppanel_test.h"

#include "remotemanagementtoppanel.h"
#include "service.h"
#include "mainwindow.h"
#include "../stub.h"

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>

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

    // 未知类型
    topPanel.m_currentPanelType = ServerConfigManager::PanelType_Search;
    topPanel.showSearchPanel("1995");
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
    topPanel.showGroupPanel("1988", true);

    topPanel.m_currentPanelType = ServerConfigManager::PanelType_Search;
    topPanel.showGroupPanel("1995", true);

    // 未知类型
    topPanel.m_currentPanelType = ServerConfigManager::PanelType_Group;
    topPanel.showGroupPanel("1995", true);
    s.reset(ADDR(MainWindow, focusCurrentPage));
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
    topPanel->m_currentPanelType = ServerConfigManager::PanelType_Manage;
    topPanel->showPrevPanel();

    // 搜索返回
    topPanel->m_currentPanelType = ServerConfigManager::PanelType_Search;
    topPanel->showPrevPanel();

    // 分组返回
    topPanel->m_currentPanelType = ServerConfigManager::PanelType_Search;
    topPanel->showPrevPanel();

    // 栈为空,最后返回都是主界面
    EXPECT_EQ(topPanel->m_currentPanelType, ServerConfigManager::PanelType_Manage);
    s.reset(ADDR(MainWindow, isFocusOnList));
}
#endif
