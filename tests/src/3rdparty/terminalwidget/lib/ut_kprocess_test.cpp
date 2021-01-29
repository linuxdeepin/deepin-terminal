#include "ut_kprocess_test.h"

#include "kprocess.h"

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QDebug>

UT_KProcess_Test::UT_KProcess_Test()
{
}

void UT_KProcess_Test::SetUp()
{
    m_process = new KProcess;
}

void UT_KProcess_Test::TearDown()
{
    delete m_process;
}

#ifdef UT_KPROCESS_TEST

/*******************************************************************************
 1. @函数:    KProcess类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    setOutputChannelMode单元测试
*******************************************************************************/
TEST_F(UT_KProcess_Test, setOutputChannelMode)
{
    m_process->setOutputChannelMode(KProcess::OnlyStdoutChannel);

    EXPECT_EQ(m_process->outputChannelMode(), KProcess::OnlyStdoutChannel);
}

/*******************************************************************************
 1. @函数:    KProcess类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    setOutputChannelMode单元测试
*******************************************************************************/
TEST_F(UT_KProcess_Test, processTest)
{
    QString program = QString("/usr/bin/ls");
    QStringList arguments;
    arguments << QString("/usr/share");
    m_process->setNextOpenMode(QIODevice::ReadWrite | QIODevice::Unbuffered | QIODevice::Text);
    m_process->setProgram(program, arguments);
    m_process->start();

    m_process->waitForFinished(-1);
}

/*******************************************************************************
 1. @函数:    KProcess类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    execute单元测试
*******************************************************************************/
TEST_F(UT_KProcess_Test, execute)
{
    QString program = QString("/usr/bin/ls");
    QStringList arguments;
    arguments << QString("/usr/share");
    m_process->setNextOpenMode(QIODevice::ReadOnly);
    m_process->setProgram(program, arguments);
    m_process->execute();
}

/*******************************************************************************
 1. @函数:    KProcess类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    execute单元测试
*******************************************************************************/
TEST_F(UT_KProcess_Test, executeWithArgs)
{
    QString program = QString("/usr/bin/ls");
    QStringList arguments;
    arguments << QString("/usr/share");
    KProcess::execute(program, arguments);
}

/*******************************************************************************
 1. @函数:    KProcess类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    execute单元测试
*******************************************************************************/
TEST_F(UT_KProcess_Test, executeOnlyProgram)
{
    QString program = QString("/usr/bin/ls");
    QStringList programList;
    programList << program;
    KProcess::execute(programList);
}

/*******************************************************************************
 1. @函数:    KProcess类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    startDetached单元测试
*******************************************************************************/
TEST_F(UT_KProcess_Test, startDetached)
{
    QString program = QString("/usr/bin/ls");
    QStringList arguments;
    arguments << QString("/usr/share");
    m_process->setNextOpenMode(QIODevice::ReadOnly);
    m_process->setProgram(program, arguments);
    m_process->startDetached();
}

/*******************************************************************************
 1. @函数:    KProcess类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    startDetached单元测试
*******************************************************************************/
TEST_F(UT_KProcess_Test, startDetachedWithArgs)
{
    QStringList arguments;
    arguments << QString("/usr/share");
    KProcess::startDetached("/usr/bin/ls", arguments);
}

/*******************************************************************************
 1. @函数:    KProcess类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    setEnv单元测试
*******************************************************************************/
TEST_F(UT_KProcess_Test, setEnv)
{
    m_process->setEnv("WINDOW_ID", "1234", true);
}

/*******************************************************************************
 1. @函数:    KProcess类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    unsetEnv单元测试
*******************************************************************************/
TEST_F(UT_KProcess_Test, unsetEnv)
{
    m_process->unsetEnv("WINDOW_ID");
}

/*******************************************************************************
 1. @函数:    KProcess类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    clearEnvironment单元测试
*******************************************************************************/
TEST_F(UT_KProcess_Test, clearEnvironment)
{
    m_process->clearEnvironment();
}

/*******************************************************************************
 1. @函数:    KProcess类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    pid单元测试
*******************************************************************************/
TEST_F(UT_KProcess_Test, pidTest)
{
    QString program = QString("/usr/bin/ls");
    QStringList arguments;
    arguments << QString("/usr/share");
    m_process->setNextOpenMode(QIODevice::ReadOnly);
    m_process->setProgram(program, arguments);
    m_process->start();

    int pid = m_process->pid();
    EXPECT_EQ(pid > 1, true);

    m_process->waitForFinished(-1);
}

#endif
