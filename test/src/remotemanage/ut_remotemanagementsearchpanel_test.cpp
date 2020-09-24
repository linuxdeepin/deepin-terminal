#include "ut_remotemanagementsearchpanel_test.h"

#define private public
#include "remotemanagementsearchpanel.h"
#include "service.h"
#include "mainwindow.h"
#undef public

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>

UT_RemoteManagementSearchPanel_Test::UT_RemoteManagementSearchPanel_Test()
{
}

void UT_RemoteManagementSearchPanel_Test::SetUp()
{
    if (!Service::instance()->property("isServiceInit").toBool())
    {
        Service::instance()->init();
        Service::instance()->setProperty("isServiceInit", true);
    }

    m_normalTermProperty[QuakeMode] = false;
    m_normalTermProperty[SingleFlag] = true;
    m_normalWindow = new NormalWindow(m_normalTermProperty, nullptr);
    m_normalWindow->resize(800, 600);
    m_normalWindow->show();

    m_normalWindow->showPlugin(MainWindow::PLUGIN_TYPE_REMOTEMANAGEMENT);
}

void UT_RemoteManagementSearchPanel_Test::TearDown()
{
    delete m_normalWindow;
}

#ifdef UT_REMOTEMANAGEMENTSEARCHPANEL_TEST

TEST_F(UT_RemoteManagementSearchPanel_Test, refreshDataByGroupAndFilter)
{
    EXPECT_EQ(m_normalWindow->isVisible(), true);

    RemoteManagementPlugin *remotePlugin = m_normalWindow->findChild<RemoteManagementPlugin *>();
    RemoteManagementTopPanel *remoteTopPanel = remotePlugin->getRemoteManagementTopPanel();
    EXPECT_NE(remoteTopPanel, nullptr);
    EXPECT_EQ(remoteTopPanel->isVisible(), true);

    RemoteManagementSearchPanel *searchPanel = remoteTopPanel->findChild<RemoteManagementSearchPanel*>();
    searchPanel->refreshDataByGroupAndFilter("group01", "server");

#ifdef ENABLE_UI_TEST
    QTest::qWait(200);
#endif
}

TEST_F(UT_RemoteManagementSearchPanel_Test, refreshDataByFilter)
{
    //void refreshDataByFilter(const QString &strFilter);
    EXPECT_EQ(m_normalWindow->isVisible(), true);

    RemoteManagementPlugin *remotePlugin = m_normalWindow->findChild<RemoteManagementPlugin *>();
    RemoteManagementTopPanel *remoteTopPanel = remotePlugin->getRemoteManagementTopPanel();
    EXPECT_NE(remoteTopPanel, nullptr);
    EXPECT_EQ(remoteTopPanel->isVisible(), true);

    RemoteManagementSearchPanel *searchPanel = remoteTopPanel->findChild<RemoteManagementSearchPanel*>();
    searchPanel->refreshDataByFilter("server");

#ifdef ENABLE_UI_TEST
    QTest::qWait(200);
#endif
}

TEST_F(UT_RemoteManagementSearchPanel_Test, clearAllFocus)
{
    EXPECT_EQ(m_normalWindow->isVisible(), true);

    RemoteManagementPlugin *remotePlugin = m_normalWindow->findChild<RemoteManagementPlugin *>();
    RemoteManagementTopPanel *remoteTopPanel = remotePlugin->getRemoteManagementTopPanel();
    EXPECT_NE(remoteTopPanel, nullptr);
    EXPECT_EQ(remoteTopPanel->isVisible(), true);

    RemoteManagementSearchPanel *searchPanel = remoteTopPanel->findChild<RemoteManagementSearchPanel*>();
    searchPanel->clearAllFocus();

#ifdef ENABLE_UI_TEST
    QTest::qWait(200);
#endif
}

TEST_F(UT_RemoteManagementSearchPanel_Test, setFocusBack)
{
    EXPECT_EQ(m_normalWindow->isVisible(), true);

    RemoteManagementPlugin *remotePlugin = m_normalWindow->findChild<RemoteManagementPlugin *>();
    RemoteManagementTopPanel *remoteTopPanel = remotePlugin->getRemoteManagementTopPanel();
    EXPECT_NE(remoteTopPanel, nullptr);
    EXPECT_EQ(remoteTopPanel->isVisible(), true);

    RemoteManagementSearchPanel *searchPanel = remoteTopPanel->findChild<RemoteManagementSearchPanel*>();
    searchPanel->setFocusBack("group01", true, 0);

#ifdef ENABLE_UI_TEST
    QTest::qWait(200);
#endif
}

TEST_F(UT_RemoteManagementSearchPanel_Test, getListIndex)
{
    EXPECT_EQ(m_normalWindow->isVisible(), true);

    RemoteManagementPlugin *remotePlugin = m_normalWindow->findChild<RemoteManagementPlugin *>();
    RemoteManagementTopPanel *remoteTopPanel = remotePlugin->getRemoteManagementTopPanel();
    EXPECT_NE(remoteTopPanel, nullptr);
    EXPECT_EQ(remoteTopPanel->isVisible(), true);

    RemoteManagementSearchPanel *searchPanel = remoteTopPanel->findChild<RemoteManagementSearchPanel*>();
    int listIndex = searchPanel->getListIndex();
    EXPECT_EQ(listIndex, -1);

#ifdef ENABLE_UI_TEST
    QTest::qWait(200);
#endif
}

#endif
