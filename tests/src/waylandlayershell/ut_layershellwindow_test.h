// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UI_LAYERSHELLWINDOW_TEST_H
#define UI_LAYERSHELLWINDOW_TEST_H

#include <gtest/gtest.h>

#include <QPointer>

class QWindow;

class UI_LayerShellWindow_Test : public ::testing::Test
{
public:
    void SetUp() override;
    void TearDown() override;

protected:
    QPointer<QWindow> m_window;
};

#endif // UI_LAYERSHELLWINDOW_TEST_H
