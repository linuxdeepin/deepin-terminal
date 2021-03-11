/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:     daizhengwen <daizhengwen@uniontech.com>
 *
 * Maintainer: daizhengwen <daizhengwen@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ut_serverconfigoptdlg_test.h"

#include "serverconfigoptdlg.h"
#include "serverconfigmanager.h"
#include "utils.h"

// DTK
#include <DApplicationHelper>

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

/*******************************************************************************
 1. @函数:    getDataTest
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-24
 4. @说明:    测试带值初始化弹窗
*******************************************************************************/
TEST_F(UT_ServerConfigOptDlg_Test, getDataTest)
{
    // 初始化测试数据
    ServerConfig config;
    config.m_serverName = "test_item";
    config.m_address = "127.0.0.1";
    config.m_userName = "dzw";
    config.m_port = "22";
    // 初始化弹窗
    ServerConfigOptDlg serverConfigDialog(ServerConfigOptDlg::SCT_MODIFY, &config);

    // 未改变数据前getData
    // 指向的不是同一片空间,所以只能比较里面的值
    ServerConfig curConfig = serverConfigDialog.getData();
    EXPECT_EQ(curConfig.m_serverName, config.m_serverName);

    // 修改弹窗内服务器名称
    serverConfigDialog.m_serverName->setText("1988");
    curConfig = serverConfigDialog.getData();
    EXPECT_EQ(curConfig.m_serverName, "1988");

    // 函数修改数据 => 还原数据
    serverConfigDialog.updataData(&config);
    curConfig = serverConfigDialog.getData();
    EXPECT_EQ(curConfig.m_serverName, config.m_serverName);
}

/*******************************************************************************
 1. @函数:    resetCurServerTest
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-24
 4. @说明:    尝试重新修改弹窗原数据
*******************************************************************************/
TEST_F(UT_ServerConfigOptDlg_Test, resetCurServerTest)
{
    // 初始化测试数据
    ServerConfig config;
    config.m_serverName = "test_item";
    config.m_address = "127.0.0.1";
    config.m_userName = "dzw";
    config.m_port = "22";
    ServerConfig config2;
    config2.m_serverName = "test_item2";
    config2.m_address = "127.0.0.1";
    config2.m_userName = "dzw";
    config2.m_port = "22";
    // 初始化弹窗
    ServerConfigOptDlg serverConfigDialog(ServerConfigOptDlg::SCT_MODIFY, &config);

    // 将数据传入弹窗,获取的指针和原数据指针应该一致
    ServerConfig *curConfig = serverConfigDialog.getCurServer();
    EXPECT_EQ(&config, curConfig);

    // 函数修改原数据 => 传入的临时数据不是指针,传入后被拷贝构造,所以还原后指针不一定相同
    // 只能对比数据是否还原
    serverConfigDialog.resetCurServer(config2);
    curConfig = serverConfigDialog.getCurServer();
    EXPECT_EQ(curConfig->m_serverName, config2.m_serverName);
}

/*******************************************************************************
 1. @函数:    lambda
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-24
 4. @说明:    测试lambda表达式
*******************************************************************************/
TEST_F(UT_ServerConfigOptDlg_Test, lambda)
{
    // 初始化弹窗
    ServerConfigOptDlg serverConfigDialog(ServerConfigOptDlg::SCT_ADD, nullptr);
    serverConfigDialog.show();

    // 切换主题
    emit DApplicationHelper::instance()->themeTypeChanged(DApplicationHelper::DarkType);
    emit DApplicationHelper::instance()->themeTypeChanged(DApplicationHelper::LightType);

    // 扩展弹窗
    emit serverConfigDialog.m_advancedOptions->click();
}
#endif
