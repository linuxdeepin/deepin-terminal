#include "ut_Pty_test.h"

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QDebug>

using namespace Konsole;

UT_Pty_Test::UT_Pty_Test()
{
}

void UT_Pty_Test::SetUp()
{
}

void UT_Pty_Test::TearDown()
{
}

#ifdef UT_PTY_TEST
TEST_F(UT_Pty_Test, FlowControlTest)
{
    Pty pty;
    const bool input = true;
    pty.setFlowControlEnabled(input);
    const bool output = pty.flowControlEnabled();
    EXPECT_EQ(output, input);
}

TEST_F(UT_Pty_Test, EraseCharTest)
{
    Pty pty;
    const char input = 'x';
    pty.setErase(input);
    const char output = pty.erase();
    EXPECT_EQ(output, input);
}

TEST_F(UT_Pty_Test, UseUtmpTest)
{
    Pty pty;
    const bool input = true;
    pty.setUseUtmp(true);
    const bool output = pty.isUseUtmp();
    EXPECT_EQ(output, input);
}

TEST_F(UT_Pty_Test, RunProgramTest)
{
    Pty pty;
    QString program = QStringLiteral("sh");
    QStringList arguments;
    arguments << program;
    QStringList environments;
    const int result = pty.start(program, arguments, environments, 0, true);

    EXPECT_EQ(result, 0);

    //没有其他进程在使用Pty, 故两个函数返回的pid相同
    EXPECT_EQ(pty.foregroundProcessGroup(), pty.processId());
}

#endif
