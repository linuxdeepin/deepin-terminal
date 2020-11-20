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

TEST_F(UT_KProcess_Test, setOutputChannelMode)
{
    m_process->setOutputChannelMode(KProcess::OnlyStdoutChannel);

    EXPECT_EQ(m_process->outputChannelMode(), KProcess::OnlyStdoutChannel);
}

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

TEST_F(UT_KProcess_Test, executeTest)
{
    QString program = QString("/usr/bin/ls");
    QStringList arguments;
    arguments << QString("/usr/share");
    m_process->setNextOpenMode(QIODevice::ReadOnly);
    m_process->setProgram(program, arguments);
    m_process->execute();
}

TEST_F(UT_KProcess_Test, executeWithArgsTest)
{
    QString program = QString("/usr/bin/ls");
    QStringList arguments;
    arguments << QString("/usr/share");
    KProcess::execute(program, arguments);
}

TEST_F(UT_KProcess_Test, executeWithArgsTest2)
{
    QString program = QString("/usr/bin/ls");
    QStringList programList;
    programList << program;
    KProcess::execute(programList);
}

TEST_F(UT_KProcess_Test, startDetachedTest)
{
    QString program = QString("/usr/bin/ls");
    QStringList arguments;
    arguments << QString("/usr/share");
    m_process->setNextOpenMode(QIODevice::ReadOnly);
    m_process->setProgram(program, arguments);
    m_process->startDetached();
}

TEST_F(UT_KProcess_Test, startDetachedWithArgsTest)
{
    QStringList arguments;
    arguments << QString("/usr/share");
    KProcess::startDetached("/usr/bin/ls", arguments);
}

TEST_F(UT_KProcess_Test, setEnvTest)
{
    m_process->setEnv("WINDOW_ID", "1234", true);
}

TEST_F(UT_KProcess_Test, unsetEnvTest)
{
    m_process->unsetEnv("WINDOW_ID");
}

TEST_F(UT_KProcess_Test, clearEnvironmentTest)
{
    m_process->clearEnvironment();
}

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
