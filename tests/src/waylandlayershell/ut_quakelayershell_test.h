// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UI_QUAKELAYERSHELL_TEST_H
#define UI_QUAKELAYERSHELL_TEST_H

#include <gtest/gtest.h>

#include <QPointer>

class QWidget;

class UI_QuakeLayerShell_Test : public ::testing::Test
{
public:
    void SetUp() override;
    void TearDown() override;

protected:
    QPointer<QWidget> m_widget;
};

#endif // UI_QUAKELAYERSHELL_TEST_H
