// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "customcommandpanel.h"
#include "customcommandoptdlg.h"
#include "ut_defines.h"
#include "shortcutmanager.h"
#include "../stub.h"
#include "ut_stub_defines.h"

#include <QDebug>

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>


#include <gtest/gtest.h>

class UT_CustomCommandPanel_Test : public ::testing::Test
{
public:
    UT_CustomCommandPanel_Test();

    //用于做一些初始化操作
    virtual void SetUp();

    //用于做一些清理操作
    virtual void TearDown();
private:
    ShortcutManager *m_scManager;
};

static QAction m_action;
static QAction *ut_m_pdlg_getCurCustomCmd() {
    return &m_action;
}

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

TEST_F(UT_CustomCommandPanel_Test, showCurSearchResult)
{
    CustomCommandPanel panel;
    panel.m_searchEdit->setText("1");
    panel.showCurSearchResult();
    EXPECT_TRUE(panel.m_searchEdit->text().count() > 0);
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
    panel.showAddCustomCommandDlg();
    panel.m_bpushButtonHaveFocus = true;
    //打桩
    Stub stub;
    stub.set(ADDR(CustomCommandOptDlg, getCurCustomCmd), ut_m_pdlg_getCurCustomCmd);

    panel.onAddCommandResponse(QDialog::Accepted);
    //打桩还原
    stub.reset(ADDR(CustomCommandOptDlg, getCurCustomCmd));

    ShortcutManager::instance()->addCustomCommand(QAction("xxx"));
    QAction *action = ShortcutManager::instance()->getCustomCommandActionList().value(0);
    panel.doCustomCommand(action->text());
}

TEST_F(UT_CustomCommandPanel_Test, onFocusOut)
{
    UT_STUB_QWIDGET_SETFOCUS_CREATE;
    UT_STUB_QWIDGET_ISVISIBLE_APPEND;

    CustomCommandPanel *panel = new CustomCommandPanel;
    UT_STUB_QWIDGET_SETFOCUS_PREPARE
    panel->onFocusOut(Qt::TabFocusReason);
    //会触发setFocus函数
    EXPECT_TRUE(UT_STUB_QWIDGET_SETFOCUS_RESULT);

    panel->m_searchEdit->setFocus();
    UT_STUB_QWIDGET_SETFOCUS_PREPARE
    panel->onFocusOut(Qt::BacktabFocusReason);
    //会触发setFocus函数
    EXPECT_TRUE(UT_STUB_QWIDGET_SETFOCUS_RESULT);

    panel->deleteLater();
}
#endif
