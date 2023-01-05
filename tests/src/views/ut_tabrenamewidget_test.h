// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_TABRENAMEWIDGET_TEST_H
#define UT_TABRENAMEWIDGET_TEST_H

#include "tabrenamewidget.h"

//Google GTest 相关头文件
#include <gtest/gtest.h>

#include <QSignalSpy>
#include <QTest>

class Ut_TabRenameWidget_Test : public ::testing::Test
{
protected:
    //用于做一些初始化操作
    virtual void SetUp();

    //用于做一些清理操作
    virtual void TearDown();

private:
    TabRenameWidget *m_renameWidgetRS = nullptr;
};

#endif // UT_TABRENAMEWIDGET_TEST_H
