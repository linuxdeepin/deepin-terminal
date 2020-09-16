#include "ut_Session_test.h"
#include "Session.h"

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QDebug>

using namespace Konsole;

UT_Session_Test::UT_Session_Test()
{
}

void UT_Session_Test::SetUp()
{
}

void UT_Session_Test::TearDown()
{
}

#ifdef UT_SESSION_TEST
TEST_F(UT_Session_Test, SessionTest)
{
    Session* session = new Session();

    // No profile loaded, nothing to run
    EXPECT_EQ(session->isRunning(), false);
    EXPECT_EQ(session->program(), QString());
    EXPECT_EQ(session->arguments(), QStringList());
    EXPECT_EQ(session->tabTitleFormat(Session::LocalTabTitle), QString());
    EXPECT_EQ(session->tabTitleFormat(Session::RemoteTabTitle), QString());

    delete session;
}

#endif
