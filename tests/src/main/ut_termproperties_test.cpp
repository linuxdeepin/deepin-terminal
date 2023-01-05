/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:     wangliang <wangliang@uniontech.com>
 *
 * Maintainer: wangliang <wangliang@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
