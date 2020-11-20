#include "ut_serverconfigoptdlg_test.h"

#include "serverconfigoptdlg.h"
#include "serverconfigmanager.h"
#include "utils.h"

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>

UT_ServerConfigOptDlg_Test::UT_ServerConfigOptDlg_Test()
{
}

void UT_ServerConfigOptDlg_Test::SetUp()
{
    ServerConfigManager *serverConfigManager = ServerConfigManager::instance();
    serverConfigManager->initServerConfig();
}

void UT_ServerConfigOptDlg_Test::TearDown()
{
}

#ifdef UT_SERVERCONFIGOPTDLG_TEST

TEST_F(UT_ServerConfigOptDlg_Test, getCurServer)
{
    ServerConfigOptDlg serverCfgDlg;
    serverCfgDlg.show();
    EXPECT_EQ(serverCfgDlg.isVisible(), true);

    ServerConfig *serverConfig = serverCfgDlg.getCurServer();
    EXPECT_EQ(serverConfig, nullptr);
}

TEST_F(UT_ServerConfigOptDlg_Test, setDelServer)
{
    ServerConfigOptDlg serverCfgDlg;
    serverCfgDlg.show();
    EXPECT_EQ(serverCfgDlg.isVisible(), true);

    serverCfgDlg.setDelServer(true);
    EXPECT_EQ(serverCfgDlg.isDelServer(), true);

    serverCfgDlg.setDelServer(false);
    EXPECT_EQ(serverCfgDlg.isDelServer(), false);
}

TEST_F(UT_ServerConfigOptDlg_Test, getServerName)
{
    ServerConfigOptDlg serverCfgDlg;
    serverCfgDlg.show();
    EXPECT_EQ(serverCfgDlg.isVisible(), true);

    QString serverName = serverCfgDlg.getServerName();
    EXPECT_EQ(serverName.isEmpty(), true);
}

TEST_F(UT_ServerConfigOptDlg_Test, updataData)
{
//    ServerConfigOptDlg serverCfgDlg;

//    QString serverName = QString("new_server_%1").arg(Utils::getRandString());
//    ServerConfig *serverConfig = serverCfgDlg.getCurServer();
//    serverConfig->m_serverName = serverName;
//    serverCfgDlg.updataData(serverConfig);

//    EXPECT_EQ(serverCfgDlg.getServerName(), serverName);
}

TEST_F(UT_ServerConfigOptDlg_Test, getData)
{
    ServerConfigOptDlg serverCfgDlg;

    ServerConfig serverConfig = serverCfgDlg.getData();
    EXPECT_EQ(serverConfig.m_serverName, serverCfgDlg.getServerName());
}

TEST_F(UT_ServerConfigOptDlg_Test, resetCurServer)
{
    ServerConfigOptDlg serverCfgDlg;

    ServerConfig serverConfig = serverCfgDlg.getData();
    serverCfgDlg.resetCurServer(serverConfig);
}

#endif
