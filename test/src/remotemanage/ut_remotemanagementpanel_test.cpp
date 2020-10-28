#include "ut_remotemanagementpanel_test.h"

#define private public
#include "remotemanagementpanel.h"
#include "utils.h"
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

    prepareData();
}

int UT_RemoteManagementPanel_Test::getServerConfigCount()
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

void UT_RemoteManagementPanel_Test::prepareData()
{
    ServerConfigManager *serverConfigManager = ServerConfigManager::instance();
    serverConfigManager->initServerConfig();

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

    QTest::qWait(30);
}

void UT_RemoteManagementPanel_Test::TearDown()
{
}

#ifdef UT_REMOTEMANAGEMENTPANEL_TEST
TEST_F(UT_RemoteManagementPanel_Test, refreshPanel)
{
    const int PANEL_WIDTH = 242;
    const int PANEL_HEIGHT = 600;
    RemoteManagementPanel panel;
    panel.resize(PANEL_WIDTH, PANEL_HEIGHT);
    panel.show();
    EXPECT_EQ(panel.size().width(), PANEL_WIDTH);
    EXPECT_EQ(panel.size().height(), PANEL_HEIGHT);

    panel.refreshPanel();

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
}

TEST_F(UT_RemoteManagementPanel_Test, setFocusInPanel)
{
    const int PANEL_WIDTH = 242;
    const int PANEL_HEIGHT = 600;
    RemoteManagementPanel panel;
    panel.resize(PANEL_WIDTH, PANEL_HEIGHT);
    panel.show();
    EXPECT_EQ(panel.size().width(), PANEL_WIDTH);
    EXPECT_EQ(panel.size().height(), PANEL_HEIGHT);

    panel.setFocusInPanel();
    int listIndex = panel.getListIndex();
    qDebug() << "listIndex:" << listIndex << endl;
    EXPECT_EQ(listIndex, -1);

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
}

TEST_F(UT_RemoteManagementPanel_Test, setFocusBack)
{
    const int PANEL_WIDTH = 242;
    const int PANEL_HEIGHT = 600;
    RemoteManagementPanel panel;
    panel.resize(PANEL_WIDTH, PANEL_HEIGHT);
    panel.show();
    EXPECT_EQ(panel.size().width(), PANEL_WIDTH);
    EXPECT_EQ(panel.size().height(), PANEL_HEIGHT);

//    panel.setFocusBack("group01");
//    int listIndex = panel.getListIndex();
//    EXPECT_GE(listIndex, 0);

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
}

TEST_F(UT_RemoteManagementPanel_Test, clearListFocus)
{
    const int PANEL_WIDTH = 242;
    const int PANEL_HEIGHT = 600;
    RemoteManagementPanel panel;
    panel.resize(PANEL_WIDTH, PANEL_HEIGHT);
    panel.show();
    EXPECT_EQ(panel.size().width(), PANEL_WIDTH);
    EXPECT_EQ(panel.size().height(), PANEL_HEIGHT);

    panel.clearListFocus();
    EXPECT_EQ(panel.m_pushButton->hasFocus(), false);
    EXPECT_EQ(panel.m_listWidget->hasFocus(), false);
    EXPECT_EQ(panel.m_searchEdit->hasFocus(), false);
    EXPECT_EQ(panel.m_listWidget->currentIndex(), -1);

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
}

TEST_F(UT_RemoteManagementPanel_Test, refreshSearchState)
{
    const int PANEL_WIDTH = 242;
    const int PANEL_HEIGHT = 600;
    RemoteManagementPanel panel;
    panel.resize(PANEL_WIDTH, PANEL_HEIGHT);
    panel.show();
    EXPECT_EQ(panel.size().width(), PANEL_WIDTH);
    EXPECT_EQ(panel.size().height(), PANEL_HEIGHT);

    panel.refreshSearchState();

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
}

#endif
