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

#include "ut_tabrenamewidget_test.h"

//用于做一些初始化操作
void Ut_TabRenameWidget_Test::SetUp()
{
    m_renameWidgetRS = new TabRenameWidget(true, true);
}

//用于做一些清理操作
void Ut_TabRenameWidget_Test::TearDown()
{
    delete m_renameWidgetRS;
}

//远程 设置界面
TEST_F(Ut_TabRenameWidget_Test, TestInitConnection)
{
    QList<QAction *> list = m_renameWidgetRS->m_choseMenu->findChildren<QAction *>();

    QAction *firstAction = list.first();
    //取出action的text为空,信号发送后程序崩溃，只能重新设置action的文字
    //也反映出槽函数没有对特殊情况处理，需要的话可以修改
    if (firstAction->text().isNull()) {
        firstAction->setText("username: %u");
    }

    m_renameWidgetRS->getInputedit()->clear();

    emit m_renameWidgetRS->m_choseMenu->triggered(list.first());
    EXPECT_TRUE(m_renameWidgetRS->getInputedit()->text() == "%u");
    EXPECT_TRUE(m_renameWidgetRS->m_Label == nullptr);
}


//普通 设置界面
TEST_F(Ut_TabRenameWidget_Test, TestNormalIsSeeting)
{
    TabRenameWidget *m_renameWidgetNS = nullptr;
    m_renameWidgetNS = new TabRenameWidget(false, true);

    EXPECT_TRUE(m_renameWidgetNS->m_Label == nullptr);

    delete m_renameWidgetNS;
}

//远程 对话框
TEST_F(Ut_TabRenameWidget_Test, TestRemateNotSeeting)
{
    TabRenameWidget *m_renameWidgetRNS = nullptr;
    m_renameWidgetRNS = new TabRenameWidget(true, false);

    EXPECT_TRUE(m_renameWidgetRNS->m_Label != nullptr);
//    EXPECT_TRUE(m_renameWidgetRNS->m_Label->text() == "Remote tab title format");

    delete m_renameWidgetRNS;
}

//非远程 对话框
TEST_F(Ut_TabRenameWidget_Test, TestNormaNotSeeting)
{
    TabRenameWidget *m_renameWidgetNNS = nullptr;
    m_renameWidgetNNS = new TabRenameWidget(false, false);

    EXPECT_TRUE(m_renameWidgetNNS->m_Label != nullptr);
//    EXPECT_TRUE(m_renameWidgetNNS->m_Label->text() == "Tab title format");

    delete m_renameWidgetNNS;
}
