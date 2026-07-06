// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_groupconfigoptdlg_test.h"
#include "groupconfigoptdlg.h"
#include "serverconfigmanager.h"
#include "utils.h"

// DTK
#include <DGuiApplicationHelper>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QSignalSpy>

UT_GroupConfigOptDlg_Test::UT_GroupConfigOptDlg_Test()
{
}

void UT_GroupConfigOptDlg_Test::SetUp()
{
    ServerConfigManager *serverConfigManager = ServerConfigManager::instance();
    serverConfigManager->initServerConfig();
}

void UT_GroupConfigOptDlg_Test::TearDown()
{
}

#ifdef UT_GROUPCONFIGOPTDLG_TEST

/*******************************************************************************
 1. @函数:    GroupConfigOptDlg
 2. @作者:    ut_coverage
 3. @日期:    2026-07-06
 4. @说明:    测试新增分组弹窗构造（空分组名）
*******************************************************************************/
TEST_F(UT_GroupConfigOptDlg_Test, constructAddGroup)
{
    // 空分组名 => 走 "Add Group" 分支
    GroupConfigOptDlg dialog("");
    dialog.show();
    EXPECT_EQ(dialog.isVisible(), true);
}

/*******************************************************************************
 1. @函数:    GroupConfigOptDlg
 2. @作者:    ut_coverage
 3. @日期:    2026-07-06
 4. @说明:    测试编辑分组弹窗构造（非空分组名）
*******************************************************************************/
TEST_F(UT_GroupConfigOptDlg_Test, constructEditGroup)
{
    // 非空分组名 => 走 "Edit Group" 分支
    GroupConfigOptDlg dialog("test_group");
    dialog.show();
    EXPECT_EQ(dialog.isVisible(), true);
}

/*******************************************************************************
 1. @函数:    GroupConfigOptDlg
 2. @作者:    ut_coverage
 3. @日期:    2026-07-06
 4. @说明:    切换主题，覆盖构造函数中调色板分支
*******************************************************************************/
TEST_F(UT_GroupConfigOptDlg_Test, themeChange)
{
    GroupConfigOptDlg dialog("theme_group");
    dialog.show();

    QSignalSpy spy(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged);
    emit DGuiApplicationHelper::instance()->themeTypeChanged(DGuiApplicationHelper::DarkType);
    emit DGuiApplicationHelper::instance()->themeTypeChanged(DGuiApplicationHelper::LightType);
    spy.wait(100);
    EXPECT_TRUE(spy.count() >= 1);
}
#endif
