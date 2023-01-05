// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_CUSTOMCOMMANDOPTDLG_TEST_H
#define UT_CUSTOMCOMMANDOPTDLG_TEST_H

#include "ut_defines.h"

#include <QString>

#include <gtest/gtest.h>

class QAction;
class UT_CustomCommandOptDlg_Test : public ::testing::Test
{
public:
    UT_CustomCommandOptDlg_Test();
    ~UT_CustomCommandOptDlg_Test();

public:
    //这里的几个函数都会自动调用

    //用于做一些初始化操作
    virtual void SetUp();

    //用于做一些清理操作
    virtual void TearDown();

    const QString m_validCmdName = "cmd";
    const QString m_validCmdText = "ls -al";
    const QString m_validCmdShortcut = "Ctrl+Shift+T";
};
#endif//UT_CUSTOMCOMMANDOPTDLG_TEST_H
