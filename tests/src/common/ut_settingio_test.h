// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_SETTINGIO_TEST_H
#define UT_SETTINGIO_TEST_H

#include "ut_defines.h"

#include <gtest/gtest.h>
/*******************************************************************************
 1. @类名:    UT_SettingIO_Test
 2. @作者:    ut000125 孙成熙
 3. @日期:    2020-12-15
 4. @说明:    SettingIO类测试
*******************************************************************************/
class UT_SettingIO_Test : public ::testing::Test
{
public:
    UT_SettingIO_Test();

public:
    //这里的几个函数都会自动调用

    //用于做一些初始化操作
    virtual void SetUp();

    //用于做一些清理操作
    virtual void TearDown();
};

#endif // UT_SETTINGIO_TEST_H
