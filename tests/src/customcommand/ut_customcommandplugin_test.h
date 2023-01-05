// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_CUSTOMCOMMANDPLUGIN_TEST_H
#define UT_CUSTOMCOMMANDPLUGIN_TEST_H

#include "ut_defines.h"

#include <gtest/gtest.h>

class CustomCommandPlugin;
class UT_Customcommandplugin_Test : public ::testing::Test
{
public:
    UT_Customcommandplugin_Test();

public:
    //这里的几个函数都会自动调用

    //用于做一些初始化操作
    virtual void SetUp();

    //用于做一些清理操作
    virtual void TearDown();
public:
    CustomCommandPlugin* m_cmdPlugin = nullptr;
};

#endif//UT_CUSTOMCOMMANDPLUGIN_TEST_H
