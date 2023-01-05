// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UI_WINDOWSMANAGER_TEST_H
#define UI_WINDOWSMANAGER_TEST_H

#include "ut_defines.h"
#include "termproperties.h"

#include <gtest/gtest.h>

class Service;
class MainWindow;
class UI_WindowsManager_Test : public ::testing::Test
{
public:
    UI_WindowsManager_Test();

public:
    //这里的几个函数都会自动调用

    //用于做一些初始化操作
    virtual void SetUp();

    //用于做一些清理操作
    virtual void TearDown();

public:
    Service *m_service = nullptr;

    TermProperties m_normalTermProperty;
    TermProperties m_quakeTermProperty;
};

#endif // UI_WINDOWSMANAGER_TEST_H

