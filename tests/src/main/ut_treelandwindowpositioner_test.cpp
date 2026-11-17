// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_treelandwindowpositioner_test.h"

#include "treelandwindowpositioner.h"

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui/qevent.h>
#include <QGuiApplication>
#include <QScreen>
#include <QWidget>

UT_TreelandWindowPositioner_Test::UT_TreelandWindowPositioner_Test()
{
}

void UT_TreelandWindowPositioner_Test::SetUp()
{
}

void UT_TreelandWindowPositioner_Test::TearDown()
{
}

#ifdef UT_TREELANDWINDOWPOSITIONER_TEST

namespace {

// 通过公开 eventFilter 让测试能够直接投递事件
class TestablePositioner : public TreelandWindowPositioner
{
public:
    using TreelandWindowPositioner::TreelandWindowPositioner;
    using TreelandWindowPositioner::eventFilter;
};

} // namespace

TEST_F(UT_TreelandWindowPositioner_Test, UnsupportedByDefault)
{
    // 测试环境为 offscreen 平台，定位器应判定为不受支持
    EXPECT_FALSE(TreelandWindowPositioner::isSupported());
}

TEST_F(UT_TreelandWindowPositioner_Test, ConstructionAndNoopApply)
{
    QWidget widget;
    TreelandWindowPositioner positioner(&widget);
    positioner.setTargetScreen(widget.screen());
    // 非 Wayland 环境不应崩溃、不应产生任何副作用
    positioner.applyPosition();
    SUCCEED();
}

TEST_F(UT_TreelandWindowPositioner_Test, ScreenForPositionInsideScreen)
{
    QWidget widget;
    QScreen *screen = widget.screen();
    ASSERT_NE(screen, nullptr);

    const QPoint inside = screen->geometry().center();
    EXPECT_EQ(TreelandWindowPositioner::screenForPosition(&widget, inside), screen);
}

TEST_F(UT_TreelandWindowPositioner_Test, ScreenForPositionFallsBackToWidgetScreen)
{
    QWidget widget;
    QScreen *screen = widget.screen();
    ASSERT_NE(screen, nullptr);

    // 光标位置远离任何屏幕时，回退到窗口所在屏幕
    const QPoint farAway(-123456, -123456);
    QScreen *cursorScreen = QGuiApplication::screenAt(farAway);
    if (!cursorScreen) {
        EXPECT_EQ(TreelandWindowPositioner::screenForPosition(&widget, farAway), screen);
    } else {
        EXPECT_EQ(TreelandWindowPositioner::screenForPosition(&widget, farAway), cursorScreen);
    }
}

TEST_F(UT_TreelandWindowPositioner_Test, ScreenForPositionWithNullWindow)
{
    QWidget widget;
    const QPoint inside = widget.screen()->geometry().center();
    // 窗口为空时仍可基于光标位置解析屏幕
    EXPECT_EQ(TreelandWindowPositioner::screenForPosition(nullptr, inside),
              QGuiApplication::screenAt(inside));
}

TEST_F(UT_TreelandWindowPositioner_Test, EventFilterDelegatesUnrelatedEvents)
{
    QWidget widget;
    TestablePositioner positioner(&widget);

    QEvent showEvent(QEvent::Show);
    EXPECT_FALSE(positioner.eventFilter(&widget, &showEvent));
}

TEST_F(UT_TreelandWindowPositioner_Test, EventFilterHandlesPlatformSurface)
{
    QWidget widget;
    TestablePositioner positioner(&widget);

    QPlatformSurfaceEvent surfaceEvent(QPlatformSurfaceEvent::SurfaceCreated);
    EXPECT_FALSE(positioner.eventFilter(&widget, &surfaceEvent));
}

#endif // UT_TREELANDWINDOWPOSITIONER_TEST
