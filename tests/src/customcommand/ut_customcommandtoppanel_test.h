// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_CUSTOMCOMMANDTOPPANEL_TEST_H
#define UT_CUSTOMCOMMANDTOPPANEL_TEST_H

#include "ut_defines.h"
#include "termproperties.h"

#include <gtest/gtest.h>

class MainWindow;
class CustomCommandTopPanel;
class UT_CustomCommandTopPanel_Test : public ::testing::Test
{
public:
    UT_CustomCommandTopPanel_Test();

public:
    //这里的几个函数都会自动调用

    //用于做一些初始化操作
    virtual void SetUp();

    //用于做一些清理操作
    virtual void TearDown();
private:
    TermProperties m_normalTermProperty;
    MainWindow *m_normalWindow = nullptr;
    CustomCommandTopPanel* m_cmdTopPanel = nullptr;
};

#endif//UT_CUSTOMCOMMANDTOPPANEL_TEST_H

