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

#include "ut_ShellCommand_test.h"
#include "ShellCommand.h"

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QDebug>

using namespace Konsole;

UT_ShellCommand_Test::UT_ShellCommand_Test()
{
}

void UT_ShellCommand_Test::SetUp()
{
}

void UT_ShellCommand_Test::TearDown()
{
}

#ifdef UT_SHELLCOMMAND_TEST

/*******************************************************************************
 1. @函数:    ShellCommand类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    command单元测试
*******************************************************************************/
TEST_F(UT_ShellCommand_Test, command)
{
    const QString fullCommand(QStringLiteral("sudo apt-get update"));
    ShellCommand shellCommand(fullCommand);
    EXPECT_EQ(shellCommand.command(), QStringLiteral("sudo"));
}

/*******************************************************************************
 1. @函数:    ShellCommand类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    arguments单元测试
*******************************************************************************/
TEST_F(UT_ShellCommand_Test, arguments)
{
    const QString command(QStringLiteral("wc"));
    QStringList arguments;
    arguments << QStringLiteral("wc") << QStringLiteral("-l") << QStringLiteral("*.cpp");

    ShellCommand shellCommand(command, arguments);
    EXPECT_EQ(shellCommand.arguments(), arguments);
}

/*******************************************************************************
 1. @函数:    ShellCommand类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    fullCommand单元测试
*******************************************************************************/
TEST_F(UT_ShellCommand_Test, fullCommand)
{
    const QString command(QStringLiteral("wc"));
    QStringList arguments;
    arguments << QStringLiteral("wc") << QStringLiteral("-l") << QStringLiteral("*.cpp");

    ShellCommand shellCommand(command, arguments);
    EXPECT_EQ(shellCommand.fullCommand(), arguments.join(QLatin1String(" ")));
}

/*******************************************************************************
 1. @函数:    ShellCommand类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    expand单元测试
*******************************************************************************/
TEST_F(UT_ShellCommand_Test, expand)
{
    QString text = QStringLiteral("$ABC '$ABC'");
    qputenv("ABC", "123");
    const QString result3 = ShellCommand::expand(text);
    const QString expected3 = QStringLiteral("123 '$ABC'");
    EXPECT_EQ(result3, expected3);
}

#endif
