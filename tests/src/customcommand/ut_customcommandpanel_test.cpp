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

#include "ut_customcommandpanel_test.h"

#include "customcommandpanel.h"

#include <QDebug>

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>

UT_CustomCommandPanel_Test::UT_CustomCommandPanel_Test():m_scManager(nullptr)
{
}

void UT_CustomCommandPanel_Test::SetUp()
{
    //快捷键、自定义命令
    m_scManager = ShortcutManager::instance();
    m_scManager->createCustomCommandsFromConfig();
}

void UT_CustomCommandPanel_Test::TearDown()
{
}

#ifdef UT_CUSTOMCOMMANDPANEL_TEST

TEST_F(UT_CustomCommandPanel_Test, CustomCommandPanelTest)
{
    QList<QAction *> cmdActionlist = m_scManager->getCustomCommandActionList();

    const int PANEL_WIDTH = 242;
    const int PANEL_HEIGHT = 600;
    CustomCommandPanel panel;
    panel.resize(PANEL_WIDTH, PANEL_HEIGHT);
    panel.show();
    EXPECT_EQ(panel.size().width(), PANEL_WIDTH);
    EXPECT_EQ(panel.size().height(), PANEL_HEIGHT);

    panel.refreshCmdPanel();

    ListView *cmdListWidget = panel.findChild<ListView*>();
    EXPECT_EQ(cmdActionlist.size(), cmdListWidget->count());

    panel.refreshCmdSearchState();
}

TEST_F(UT_CustomCommandPanel_Test, showAddCustomCommandDlg)
{
    QList<QAction *> cmdActionlist = m_scManager->getCustomCommandActionList();

    const int PANEL_WIDTH = 242;
    const int PANEL_HEIGHT = 600;
    CustomCommandPanel panel;
    panel.resize(PANEL_WIDTH, PANEL_HEIGHT);
    panel.show();
    EXPECT_EQ(panel.size().width(), PANEL_WIDTH);
    EXPECT_EQ(panel.size().height(), PANEL_HEIGHT);

    panel.refreshCmdPanel();

    ListView *cmdListWidget = panel.findChild<ListView*>();
    EXPECT_EQ(cmdActionlist.size(), cmdListWidget->count());

    panel.refreshCmdSearchState();
    panel.showAddCustomCommandDlg();
}

#endif
