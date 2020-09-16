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
TEST_F(UT_ShellCommand_Test, ShellCommandTest)
{
    const QString fullCommand(QStringLiteral("sudo apt-get update"));
    ShellCommand shellCommand(fullCommand);
    EXPECT_EQ(shellCommand.command(), QStringLiteral("sudo"));
    EXPECT_EQ(shellCommand.fullCommand(), fullCommand);
}

TEST_F(UT_ShellCommand_Test, ConstructorWithTwoArgumentsTest)
{
    const QString command(QStringLiteral("wc"));
    QStringList arguments;
    arguments << QStringLiteral("wc") << QStringLiteral("-l") << QStringLiteral("*.cpp");

    ShellCommand shellCommand(command, arguments);
    EXPECT_EQ(shellCommand.command(), command);
    EXPECT_EQ(shellCommand.arguments(), arguments);
    EXPECT_EQ(shellCommand.fullCommand(), arguments.join(QLatin1String(" ")));
}

TEST_F(UT_ShellCommand_Test, ExpandEnvironmentVariableTest)
{
    QString text = QStringLiteral("$ABC '$ABC'");
    qputenv("ABC", "123");
    const QString result3 = ShellCommand::expand(text);
    const QString expected3 = QStringLiteral("123 '$ABC'");
    EXPECT_EQ(result3, expected3);
}

TEST_F(UT_ShellCommand_Test, EmptyCommandTest)
{
    const QString command = QString();
    ShellCommand shellCommand(command);
    EXPECT_EQ(shellCommand.command(), QString());
    EXPECT_EQ(shellCommand.arguments(), QStringList());
    EXPECT_EQ(shellCommand.fullCommand(), QString());
}

#endif
