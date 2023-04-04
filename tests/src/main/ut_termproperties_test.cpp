// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_termproperties_test.h"

#include "termproperties.h"

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>

UT_TermProperties_Test::UT_TermProperties_Test()
{
}

void UT_TermProperties_Test::SetUp()
{
}

void UT_TermProperties_Test::TearDown()
{
}

#ifdef UT_TERMPROPERTIES_TEST

TEST_F(UT_TermProperties_Test, setWorkingDir)
{
    QString workingDir = "";
    TermProperties tp(workingDir);
    workingDir = "/home/uos";
    tp.setWorkingDir(workingDir);

    EXPECT_EQ(tp.m_properties[TermProperty::WorkingDir], workingDir);
}

TEST_F(UT_TermProperties_Test, contains)
{
    QString workingDir = "/home/uos";
    QString colorScheme = "someColorScheme";
    TermProperties tp(workingDir, colorScheme);
    EXPECT_EQ(tp.contains(TermProperty::WorkingDir), true);
    EXPECT_EQ(tp.contains(TermProperty::ColorScheme), true);
}

TEST_F(UT_TermProperties_Test, setTermPropertyMap)
{
    QMap<TermProperty, QVariant> list;
    TermProperties tp(list);
    QString workingDir = "/home/uos";
    QString colorScheme = "someColorScheme";

    list.insert(TermProperty::WorkingDir, workingDir);
    list.insert(TermProperty::ColorScheme, colorScheme);

    tp.setTermPropertyMap(list);

    EXPECT_EQ(tp.m_properties, list);
}
#endif
