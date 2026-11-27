// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_quakelayershell_test.h"

#include "quakelayershell.h"

#include <QGuiApplication>
#include <QScreen>
#include <QWidget>

using TerminalLayerShell::QuakeLayerShell;

void UI_QuakeLayerShell_Test::SetUp()
{
    m_widget = new QWidget;
}

void UI_QuakeLayerShell_Test::TearDown()
{
    delete m_widget;
    m_widget = nullptr;
}

TEST_F(UI_QuakeLayerShell_Test, attachFailsOutsideWayland)
{
    // 测试进程使用 offscreen 平台，layer-shell 必须保持不可用
    QuakeLayerShell shell(m_widget);

    EXPECT_FALSE(shell.attach());
    EXPECT_FALSE(shell.isActive());
    EXPECT_FALSE(shell.isMapped());
}

TEST_F(UI_QuakeLayerShell_Test, mapAndUnmapFallbackTracksVisibility)
{
    QuakeLayerShell shell(m_widget);
    ASSERT_FALSE(shell.attach());

    shell.map();
    EXPECT_TRUE(shell.isMapped());
    EXPECT_TRUE(m_widget->isVisible());

    // 重复 map 不应改变状态
    shell.map();
    EXPECT_TRUE(shell.isMapped());

    shell.unmap();
    EXPECT_FALSE(shell.isMapped());
    EXPECT_FALSE(m_widget->isVisible());

    // 重复 unmap 是空操作
    shell.unmap();
    EXPECT_FALSE(shell.isMapped());
    EXPECT_FALSE(m_widget->isVisible());

    // 再次显示走同一回退路径
    shell.map();
    EXPECT_TRUE(shell.isMapped());
    EXPECT_TRUE(m_widget->isVisible());
}

TEST_F(UI_QuakeLayerShell_Test, screenAndHeightFallback)
{
    QuakeLayerShell shell(m_widget);
    ASSERT_FALSE(shell.attach());

    QScreen *screen = QGuiApplication::primaryScreen();
    ASSERT_NE(screen, nullptr);

    shell.setScreen(screen);
    EXPECT_EQ(shell.screen(), screen);

    // nullptr 被忽略
    shell.setScreen(nullptr);
    EXPECT_EQ(shell.screen(), screen);

    shell.updateHeight(240);
    EXPECT_EQ(m_widget->height(), 240);

    // 非正高度被忽略
    shell.updateHeight(0);
    EXPECT_EQ(m_widget->height(), 240);
    shell.updateHeight(-10);
    EXPECT_EQ(m_widget->height(), 240);
}
