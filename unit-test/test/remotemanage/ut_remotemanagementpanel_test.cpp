#include "ut_remotemanagementpanel_test.h"

#include "remotemanagementpanel.h"

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>

UT_RemoteManagementPanel_Test::UT_RemoteManagementPanel_Test()
{
}

void UT_RemoteManagementPanel_Test::SetUp()
{
    //远程服务器管理
    m_serverConfigManager = ServerConfigManager::instance();
}

void UT_RemoteManagementPanel_Test::TearDown()
{
}

TEST_F(UT_RemoteManagementPanel_Test, RemoteManagementPanelTest)
{
    const int PANEL_WIDTH = 242;
    const int PANEL_HEIGHT = 600;
    RemoteManagementPanel panel;
    panel.resize(PANEL_WIDTH, PANEL_HEIGHT);
    panel.show();
    EXPECT_EQ(panel.size().width(), PANEL_WIDTH);
    EXPECT_EQ(panel.size().height(), PANEL_HEIGHT);

    panel.refreshPanel();

    panel.refreshSearchState();
}
