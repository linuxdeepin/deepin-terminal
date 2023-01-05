// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_CUSTOMTHEMESETTINGDIALOG_TEST_H
#define UT_CUSTOMTHEMESETTINGDIALOG_TEST_H

#include "ut_defines.h"

#include <gtest/gtest.h>

class CustomThemeSettingDialog;
class QObject;


class UT_CustomThemeSettingDialog_Test : public ::testing::Test
{
public:
    UT_CustomThemeSettingDialog_Test();
    //这里的几个函数都会自动调用

    //用于做一些初始化操作
    virtual void SetUp();

    //用于做一些清理操作
    virtual void TearDown();

public:
    QObject *ut_object_sender_foregroundButton();


    CustomThemeSettingDialog *dialog = nullptr;
};

#endif // UT_CUSTOMTHEMESETTINGDIALOG_TEST_H
