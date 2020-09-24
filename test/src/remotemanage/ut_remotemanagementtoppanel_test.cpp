#include "ut_remotemanagementtoppanel_test.h"

#define private public
#include "remotemanagementtoppanel.h"
#include "service.h"
#include "mainwindow.h"
#undef public

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>

UT_RemoteManagementTopPanel_Test::UT_RemoteManagementTopPanel_Test()
{
}

void UT_RemoteManagementTopPanel_Test::SetUp()
{
    if (!Service::instance()->property("isServiceInit").toBool())
    {
        Service::instance()->init();
        Service::instance()->setProperty("isServiceInit", true);
    }
}

void UT_RemoteManagementTopPanel_Test::TearDown()
{
}

#ifdef UT_REMOTEMANAGEMENTTOPPANEL_TEST

TEST_F(UT_RemoteManagementTopPanel_Test, setFocusInPanel)
{
    m_normalTermProperty[QuakeMode] = false;
    m_normalTermProperty[SingleFlag] = true;
    m_normalWindow = new NormalWindow(m_normalTermProperty, nullptr);
    m_normalWindow->resize(800, 600);
    m_normalWindow->show();
    EXPECT_EQ(m_normalWindow->isVisible(), true);

    m_normalWindow->showPlugin(MainWindow::PLUGIN_TYPE_REMOTEMANAGEMENT);

    RemoteManagementPlugin *remotePlugin = m_normalWindow->findChild<RemoteManagementPlugin *>();
    RemoteManagementTopPanel *remoteTopPanel = remotePlugin->getRemoteManagementTopPanel();
    EXPECT_NE(remoteTopPanel, nullptr);
    EXPECT_EQ(remoteTopPanel->isVisible(), true);

    remoteTopPanel->setFocusInPanel();

#ifdef ENABLE_UI_TEST
    QTest::qWait(200);
#endif
    m_normalWindow->close();
    delete m_normalWindow;
}

TEST_F(UT_RemoteManagementTopPanel_Test, showSearchPanel)
{
    m_normalTermProperty[QuakeMode] = false;
    m_normalTermProperty[SingleFlag] = true;
    m_normalWindow = new NormalWindow(m_normalTermProperty, nullptr);
    m_normalWindow->resize(800, 600);
    m_normalWindow->show();
    EXPECT_EQ(m_normalWindow->isVisible(), true);

    m_normalWindow->showPlugin(MainWindow::PLUGIN_TYPE_REMOTEMANAGEMENT);

    RemoteManagementPlugin *remotePlugin = m_normalWindow->findChild<RemoteManagementPlugin *>();
    RemoteManagementTopPanel *remoteTopPanel = remotePlugin->getRemoteManagementTopPanel();
    EXPECT_NE(remoteTopPanel, nullptr);
    EXPECT_EQ(remoteTopPanel->isVisible(), true);

    remoteTopPanel->showSearchPanel("group");

#ifdef ENABLE_UI_TEST
    QTest::qWait(200);
#endif
    m_normalWindow->close();
    delete m_normalWindow;
}

TEST_F(UT_RemoteManagementTopPanel_Test, showGroupPanel)
{
    m_normalTermProperty[QuakeMode] = false;
    m_normalTermProperty[SingleFlag] = true;
    m_normalWindow = new NormalWindow(m_normalTermProperty, nullptr);
    m_normalWindow->resize(800, 600);
    m_normalWindow->show();
    EXPECT_EQ(m_normalWindow->isVisible(), true);

    m_normalWindow->showPlugin(MainWindow::PLUGIN_TYPE_REMOTEMANAGEMENT);

    RemoteManagementPlugin *remotePlugin = m_normalWindow->findChild<RemoteManagementPlugin *>();
    RemoteManagementTopPanel *remoteTopPanel = remotePlugin->getRemoteManagementTopPanel();
    EXPECT_NE(remoteTopPanel, nullptr);
    EXPECT_EQ(remoteTopPanel->isVisible(), true);

    remoteTopPanel->showSearchPanel("group");

#ifdef ENABLE_UI_TEST
    QTest::qWait(200);
#endif
    //显示前一个界面（返回）
    remoteTopPanel->showPrevPanel();

#ifdef ENABLE_UI_TEST
    QTest::qWait(200);
#endif
    m_normalWindow->close();
    delete m_normalWindow;
}
#endif
