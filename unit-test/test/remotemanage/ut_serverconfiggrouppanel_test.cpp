#include "ut_serverconfiggrouppanel_test.h"

#define private public
#include "serverconfiggrouppanel.h"
#include "service.h"
#include "mainwindow.h"
#undef public

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
    if (!Service::instance()->property("isServiceInit").toBool())
    {
        Service::instance()->init();
        Service::instance()->setProperty("isServiceInit", true);
    }

    m_normalTermProperty[QuakeMode] = false;
    m_normalTermProperty[SingleFlag] = true;
    m_normalWindow = new NormalWindow(m_normalTermProperty, nullptr);
}

void UT_ServerConfigGroupPanel_Test::TearDown()
{
    delete m_normalWindow;
}

#ifdef UT_SERVERCONFIGGROUPPANEL_TEST
TEST_F(UT_ServerConfigGroupPanel_Test, ServerConfigGroupPanelTest)
{
    m_normalWindow->resize(800, 600);
    m_normalWindow->show();
    EXPECT_EQ(m_normalWindow->isVisible(), true);

    m_normalWindow->showPlugin(MainWindow::PLUGIN_TYPE_REMOTEMANAGEMENT);

    RemoteManagementPlugin *remotePlugin = m_normalWindow->findChild<RemoteManagementPlugin *>();
    RemoteManagementTopPanel *remoteTopPanel = remotePlugin->getRemoteManagementTopPanel();
    EXPECT_NE(remoteTopPanel, nullptr);
    EXPECT_EQ(remoteTopPanel->isVisible(), true);

    ServerConfigGroupPanel *groupPanel = remoteTopPanel->findChild<ServerConfigGroupPanel *>();
    EXPECT_NE(groupPanel, nullptr);

#ifdef ENABLE_UI_TEST
    QTest::qWait(1000);
#endif
}
#endif
