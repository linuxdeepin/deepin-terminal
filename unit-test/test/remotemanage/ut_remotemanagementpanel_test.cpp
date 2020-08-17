#include "ut_remotemanagementpanel_test.h"

#define private public
#include "remotemanagementpanel.h"
#undef public

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
    // 初始化远程管理数据
    m_serverConfigManager->initServerConfig();
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

    panel.clearListFocus();
    EXPECT_EQ(panel.m_pushButton->hasFocus(), false);
    EXPECT_EQ(panel.m_listWidget->hasFocus(), false);
    EXPECT_EQ(panel.m_searchEdit->hasFocus(), false);
    EXPECT_EQ(panel.m_listWidget->currentIndex(), -1);

    panel.refreshSearchState();

#ifdef ENABLE_UI_TEST
    QTest::qWait(1000);
#endif
}
