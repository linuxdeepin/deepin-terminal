#include "ut_serverconfiggrouppanel_test.h"

#include "serverconfiggrouppanel.h"
#include "service.h"
#include "mainwindow.h"

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>

UT_ServerConfigGroupPanel_Test::UT_ServerConfigGroupPanel_Test()
{
}

void UT_ServerConfigGroupPanel_Test::SetUp()
{
    if (!Service::instance()->property("isServiceInit").toBool()) {
        Service::instance()->init();
        Service::instance()->setProperty("isServiceInit", true);
    }

    m_normalTermProperty[QuakeMode] = false;
    m_normalTermProperty[SingleFlag] = true;
    m_normalWindow = new NormalWindow(m_normalTermProperty, nullptr);
    m_normalWindow->resize(800, 600);
    m_normalWindow->show();
}

void UT_ServerConfigGroupPanel_Test::TearDown()
{
    delete m_normalWindow;
}

#ifdef UT_SERVERCONFIGGROUPPANEL_TEST
TEST_F(UT_ServerConfigGroupPanel_Test, refreshData)
{
    EXPECT_EQ(m_normalWindow->isVisible(), true);

    m_normalWindow->showPlugin(MainWindow::PLUGIN_TYPE_REMOTEMANAGEMENT);

    RemoteManagementPlugin *remotePlugin = m_normalWindow->findChild<RemoteManagementPlugin *>();
    RemoteManagementTopPanel *remoteTopPanel = remotePlugin->getRemoteManagementTopPanel();
    EXPECT_NE(remoteTopPanel, nullptr);
    EXPECT_EQ(remoteTopPanel->isVisible(), true);

    ServerConfigGroupPanel *groupPanel = remoteTopPanel->findChild<ServerConfigGroupPanel *>();
    EXPECT_NE(groupPanel, nullptr);
    groupPanel->refreshData("group01");

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
}

TEST_F(UT_ServerConfigGroupPanel_Test, setFocusBack)
{
    EXPECT_EQ(m_normalWindow->isVisible(), true);

    m_normalWindow->showPlugin(MainWindow::PLUGIN_TYPE_REMOTEMANAGEMENT);

    RemoteManagementPlugin *remotePlugin = m_normalWindow->findChild<RemoteManagementPlugin *>();
    RemoteManagementTopPanel *remoteTopPanel = remotePlugin->getRemoteManagementTopPanel();
    EXPECT_NE(remoteTopPanel, nullptr);
    EXPECT_EQ(remoteTopPanel->isVisible(), true);

    ServerConfigGroupPanel *groupPanel = remoteTopPanel->findChild<ServerConfigGroupPanel *>();
    EXPECT_NE(groupPanel, nullptr);
    groupPanel->setFocusBack();

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
}

TEST_F(UT_ServerConfigGroupPanel_Test, clearAllFocus)
{
    EXPECT_EQ(m_normalWindow->isVisible(), true);

    m_normalWindow->showPlugin(MainWindow::PLUGIN_TYPE_REMOTEMANAGEMENT);

    RemoteManagementPlugin *remotePlugin = m_normalWindow->findChild<RemoteManagementPlugin *>();
    RemoteManagementTopPanel *remoteTopPanel = remotePlugin->getRemoteManagementTopPanel();
    EXPECT_NE(remoteTopPanel, nullptr);
    EXPECT_EQ(remoteTopPanel->isVisible(), true);

    ServerConfigGroupPanel *groupPanel = remoteTopPanel->findChild<ServerConfigGroupPanel *>();
    EXPECT_NE(groupPanel, nullptr);
    groupPanel->clearAllFocus();

    EXPECT_EQ(groupPanel->m_rebackButton->hasFocus(), false);
    EXPECT_EQ(groupPanel->m_searchEdit->hasFocus(), false);
    EXPECT_EQ(groupPanel->m_listWidget->hasFocus(), false);

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
}

#endif
