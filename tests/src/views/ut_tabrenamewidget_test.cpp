// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
