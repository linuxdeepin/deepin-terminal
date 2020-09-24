#include "ut_serverconfigmanager_test.h"

#define private public
#include "serverconfigmanager.h"
#include "service.h"
#include "mainwindow.h"
#include "utils.h"
#undef public

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>

UT_ServerConfigManager_Test::UT_ServerConfigManager_Test()
{
}

void UT_ServerConfigManager_Test::SetUp()
{
}

void UT_ServerConfigManager_Test::TearDown()
{
}

int UT_ServerConfigManager_Test::getServerConfigCount()
{
    QList<ServerConfig *> serverConfigList;
    QMap<QString, QList<ServerConfig *>> severConfigs = ServerConfigManager::instance()->getServerConfigs();
    for (QMap<QString, QList<ServerConfig *>>::iterator iter = severConfigs.begin(); iter != severConfigs.end(); iter++) {
        QList<ServerConfig *> value = iter.value();
        for (int i = 0; i < value.size(); i++) {
            serverConfigList.append(value.at(i));
        }
    }

    qDebug() << serverConfigList.count();
    return serverConfigList.count();
}

#ifdef UT_SERVERCONFIGMANAGER_TEST
TEST_F(UT_ServerConfigManager_Test, ServerConfigManagerTest)
{
    ServerConfigManager *serverConfigManager = ServerConfigManager::instance();
    serverConfigManager->initServerConfig();

    QDir serverConfigBasePath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    EXPECT_EQ(serverConfigBasePath.exists(), true);

    QString serverConfigFilePath(serverConfigBasePath.filePath("server-config.conf"));
    EXPECT_EQ(QFile::exists(serverConfigFilePath), true);

    int serverConfigCount = getServerConfigCount();
    qDebug() << serverConfigCount << endl;

    QString groupName = QString("group_01");

    qsrand(static_cast<uint>(time(nullptr)));
    ServerConfig *config = new ServerConfig();
    config->m_serverName = QString("new_server_%1").arg(Utils::getRandString());
    config->m_address = QString("192.168.10.%1").arg(qrand() % 255);
    config->m_userName = QString("zhangsan");
    config->m_password = QString("123");
    config->m_privateKey = QString("");
    config->m_port = QString("");
    config->m_group = groupName;
    config->m_path = QString("");
    config->m_command = QString("");
    config->m_encoding = QString("");
    config->m_backspaceKey = QString("");
    config->m_deleteKey = QString("");

    serverConfigManager->saveServerConfig(config);
    EXPECT_EQ(getServerConfigCount(), serverConfigCount+1);

    int serverCount = serverConfigManager->getServerCount(config->m_group);

    ServerConfig *currConfig = serverConfigManager->getServerConfig(config->m_serverName);
    EXPECT_NE(currConfig, nullptr);

    QTest::qWait(30);

    qsrand(static_cast<uint>(time(nullptr)));
    ServerConfig *newConfig = new ServerConfig();
    newConfig->m_serverName = QString("new_server_%1").arg(Utils::getRandString());
    newConfig->m_address = QString("192.168.10.%1").arg(qrand() % 255);
    newConfig->m_userName = QString("uos");
    newConfig->m_password = QString("123456");
    newConfig->m_privateKey = QString("");
    newConfig->m_port = QString("");
    newConfig->m_group = groupName;
    newConfig->m_path = QString("");
    newConfig->m_command = QString("");
    newConfig->m_encoding = QString("");
    newConfig->m_backspaceKey = QString("");
    newConfig->m_deleteKey = QString("");
    serverConfigManager->modifyServerConfig(newConfig, currConfig);
    EXPECT_EQ(newConfig, serverConfigManager->getServerConfig(newConfig->m_serverName));

    EXPECT_GE(serverCount, 1);

    serverConfigManager->delServerConfig(newConfig);
    EXPECT_EQ(getServerConfigCount(), serverConfigCount);

    EXPECT_EQ(serverConfigManager->getServerCount(groupName), serverCount-1);

#ifdef ENABLE_UI_TEST
    QTest::qWait(200);
#endif
}
#endif
