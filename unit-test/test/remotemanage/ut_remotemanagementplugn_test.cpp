#include "ut_remotemanagementplugn_test.h"

#define private public
#include "remotemanagementplugn.h"
#include "mainwindow.h"
#include "service.h"
#undef public

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>

UT_RemoteManagementPlugn_Test::UT_RemoteManagementPlugn_Test()
{
}

void UT_RemoteManagementPlugn_Test::SetUp()
{
    if (!Service::instance()->property("isServiceInit").toBool())
    {
        Service::instance()->init();
        Service::instance()->setProperty("isServiceInit", true);
    }

    m_normalTermProperty[QuakeMode] = false;
    m_normalTermProperty[SingleFlag] = true;
    m_normalWindow = new NormalWindow(m_normalTermProperty, nullptr);
}

void UT_RemoteManagementPlugn_Test::TearDown()
{
    delete m_normalWindow;
}

#ifdef UT_REMOTEMANAGEMENTPLUGN_TEST
TEST_F(UT_RemoteManagementPlugn_Test, RemoteManagementPlugnTest)
{
    m_normalWindow->resize(800, 600);
    m_normalWindow->show();
    EXPECT_EQ(m_normalWindow->isVisible(), true);

    m_normalWindow->showPlugin(MainWindow::PLUGIN_TYPE_REMOTEMANAGEMENT);

    RemoteManagementPlugin *remotePlugin = m_normalWindow->findChild<RemoteManagementPlugin *>();
    RemoteManagementTopPanel *remoteTopPanel = remotePlugin->getRemoteManagementTopPanel();
    EXPECT_NE(remoteTopPanel, nullptr);
    EXPECT_EQ(remoteTopPanel->isVisible(), true);

#ifdef ENABLE_UI_TEST
    QTest::qWait(1000);
#endif
    remotePlugin->hidePlugn();
    EXPECT_EQ(remoteTopPanel->isVisible(), false);

#ifdef ENABLE_UI_TEST
    QTest::qWait(1000);
#endif
}
#endif
