// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_layershellwindow_test.h"

#include "layershellwindow.h"

#include <QSignalSpy>
#include <QWindow>

using TerminalLayerShell::LayerShellWindow;

void UI_LayerShellWindow_Test::SetUp()
{
    m_window = new QWindow;
}

void UI_LayerShellWindow_Test::TearDown()
{
    delete m_window;
    m_window = nullptr;
}

TEST_F(UI_LayerShellWindow_Test, getReturnsSameInstance)
{
    LayerShellWindow *first = LayerShellWindow::get(m_window);
    LayerShellWindow *second = LayerShellWindow::get(m_window);

    ASSERT_NE(first, nullptr);
    EXPECT_EQ(first, second);
    EXPECT_EQ(first->window(), m_window.data());
}

TEST_F(UI_LayerShellWindow_Test, inactiveOnNonWaylandPlatform)
{
    // 测试在 offscreen 平台运行，没有 QWaylandWindow，必须保持未激活状态
    LayerShellWindow *window = LayerShellWindow::get(m_window);

    ASSERT_NE(window, nullptr);
    EXPECT_FALSE(window->isActive());
}

TEST_F(UI_LayerShellWindow_Test, propertiesRoundTrip)
{
    LayerShellWindow *window = LayerShellWindow::get(m_window);

    window->setAnchors(LayerShellWindow::Anchors(LayerShellWindow::AnchorTop
                                                 | LayerShellWindow::AnchorLeft
                                                 | LayerShellWindow::AnchorRight));
    window->setExclusiveZone(0);
    window->setLayer(LayerShellWindow::LayerTop);
    window->setKeyboardInteractivity(LayerShellWindow::KeyboardInteractivityOnDemand);
    window->setScope(QStringLiteral("deepin-terminal/quake"));
    window->setCloseOnDismissed(false);
    window->setScreenConfiguration(LayerShellWindow::ScreenFromQWindow);
    window->setLeftMargin(1);
    window->setRightMargin(2);
    window->setTopMargin(3);
    window->setBottomMargin(4);
    window->setInputRegion(QRegion(0, 0, 10, 10));

    EXPECT_EQ(window->anchors(), LayerShellWindow::Anchors(LayerShellWindow::AnchorTop
                                                          | LayerShellWindow::AnchorLeft
                                                          | LayerShellWindow::AnchorRight));
    EXPECT_EQ(window->exclusionZone(), 0);
    EXPECT_EQ(window->layer(), LayerShellWindow::LayerTop);
    EXPECT_EQ(window->keyboardInteractivity(), LayerShellWindow::KeyboardInteractivityOnDemand);
    EXPECT_EQ(window->scope(), QStringLiteral("deepin-terminal/quake"));
    EXPECT_FALSE(window->closeOnDismissed());
    EXPECT_EQ(window->screenConfiguration(), LayerShellWindow::ScreenFromQWindow);
    EXPECT_EQ(window->leftMargin(), 1);
    EXPECT_EQ(window->rightMargin(), 2);
    EXPECT_EQ(window->topMargin(), 3);
    EXPECT_EQ(window->bottomMargin(), 4);
    EXPECT_EQ(window->inputRegion(), QRegion(0, 0, 10, 10));

    window->resetInputRegion();
    EXPECT_TRUE(window->inputRegion().isEmpty());
}

TEST_F(UI_LayerShellWindow_Test, changeSignalsEmittedOnce)
{
    LayerShellWindow *window = LayerShellWindow::get(m_window);

    QSignalSpy anchorsSpy(window, &LayerShellWindow::anchorsChanged);
    QSignalSpy marginsSpy(window, &LayerShellWindow::marginsChanged);
    QSignalSpy exclusiveZoneSpy(window, &LayerShellWindow::exclusionZoneChanged);
    QSignalSpy keyboardSpy(window, &LayerShellWindow::keyboardInteractivityChanged);
    QSignalSpy layerSpy(window, &LayerShellWindow::layerChanged);
    QSignalSpy scopeSpy(window, &LayerShellWindow::scopeChanged);
    QSignalSpy inputRegionSpy(window, &LayerShellWindow::inputRegionChanged);

    window->setAnchors(LayerShellWindow::AnchorTop);
    window->setAnchors(LayerShellWindow::AnchorTop);
    EXPECT_EQ(anchorsSpy.count(), 1);

    window->setTopMargin(5);
    window->setTopMargin(5);
    EXPECT_EQ(marginsSpy.count(), 1);

    window->setExclusiveZone(2);
    window->setExclusiveZone(2);
    EXPECT_EQ(exclusiveZoneSpy.count(), 1);

    window->setKeyboardInteractivity(LayerShellWindow::KeyboardInteractivityExclusive);
    window->setKeyboardInteractivity(LayerShellWindow::KeyboardInteractivityExclusive);
    EXPECT_EQ(keyboardSpy.count(), 1);

    window->setLayer(LayerShellWindow::LayerOverlay);
    window->setLayer(LayerShellWindow::LayerOverlay);
    EXPECT_EQ(layerSpy.count(), 1);

    window->setScope(QStringLiteral("scope-a"));
    window->setScope(QStringLiteral("scope-a"));
    EXPECT_EQ(scopeSpy.count(), 1);

    window->setInputRegion(QRegion(1, 1, 2, 2));
    window->setInputRegion(QRegion(1, 1, 2, 2));
    EXPECT_EQ(inputRegionSpy.count(), 1);
}
