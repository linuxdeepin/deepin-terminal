#include "ut_remotemanagementplugn_test.h"

#include "remotemanagementplugn.h"
#include "mainwindow.h"
#include "service.h"

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>

UT_RemoteManagementPlugin_Test::UT_RemoteManagementPlugin_Test()
{
}

void UT_RemoteManagementPlugin_Test::SetUp()
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

void UT_RemoteManagementPlugin_Test::TearDown()
{
    delete m_normalWindow;
}

#ifdef UT_REMOTEMANAGEMENTPLUGIN_TEST
TEST_F(UT_RemoteManagementPlugin_Test, setRemoteEncode)
{
    EXPECT_EQ(m_normalWindow->isVisible(), true);

    m_normalWindow->showPlugin(MainWindow::PLUGIN_TYPE_REMOTEMANAGEMENT);

    RemoteManagementPlugin *remotePlugin = m_normalWindow->findChild<RemoteManagementPlugin *>();
    RemoteManagementTopPanel *remoteTopPanel = remotePlugin->getRemoteManagementTopPanel();
    EXPECT_NE(remoteTopPanel, nullptr);
    EXPECT_EQ(remoteTopPanel->isVisible(), true);

    remotePlugin->setRemoteEncode(QString("UTF-8"));

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
}

TEST_F(UT_RemoteManagementPlugin_Test, setBackspaceKey)
{
    EXPECT_EQ(m_normalWindow->isVisible(), true);

    m_normalWindow->showPlugin(MainWindow::PLUGIN_TYPE_REMOTEMANAGEMENT);

    RemoteManagementPlugin *remotePlugin = m_normalWindow->findChild<RemoteManagementPlugin *>();
    RemoteManagementTopPanel *remoteTopPanel = remotePlugin->getRemoteManagementTopPanel();
    EXPECT_NE(remoteTopPanel, nullptr);
    EXPECT_EQ(remoteTopPanel->isVisible(), true);

    TermWidget *termWidget = m_normalWindow->currentPage()->currentTerminal();
    remotePlugin->setBackspaceKey(termWidget, QString("control-h"));
    remotePlugin->setBackspaceKey(termWidget, QString("auto"));
    remotePlugin->setBackspaceKey(termWidget, QString("escape-sequence"));
    remotePlugin->setBackspaceKey(termWidget, QString("ascii-del"));
    remotePlugin->setBackspaceKey(termWidget, QString("tty"));
#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
}

TEST_F(UT_RemoteManagementPlugin_Test, setDeleteKey)
{
    EXPECT_EQ(m_normalWindow->isVisible(), true);

    m_normalWindow->showPlugin(MainWindow::PLUGIN_TYPE_REMOTEMANAGEMENT);

    RemoteManagementPlugin *remotePlugin = m_normalWindow->findChild<RemoteManagementPlugin *>();
    RemoteManagementTopPanel *remoteTopPanel = remotePlugin->getRemoteManagementTopPanel();
    EXPECT_NE(remoteTopPanel, nullptr);
    EXPECT_EQ(remoteTopPanel->isVisible(), true);

    TermWidget *termWidget = m_normalWindow->currentPage()->currentTerminal();
    remotePlugin->setDeleteKey(termWidget, QString("control-h"));
    remotePlugin->setDeleteKey(termWidget, QString("auto"));
    remotePlugin->setDeleteKey(termWidget, QString("escape-sequence"));
    remotePlugin->setDeleteKey(termWidget, QString("ascii-del"));
    remotePlugin->setDeleteKey(termWidget, QString("tty"));
#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
}

TEST_F(UT_RemoteManagementPlugin_Test, createShellFile)
{
    m_normalWindow->showPlugin(MainWindow::PLUGIN_TYPE_REMOTEMANAGEMENT);

    RemoteManagementPlugin *remotePlugin = m_normalWindow->findChild<RemoteManagementPlugin *>();
    RemoteManagementTopPanel *remoteTopPanel = remotePlugin->getRemoteManagementTopPanel();
    EXPECT_NE(remoteTopPanel, nullptr);
    EXPECT_EQ(remoteTopPanel->isVisible(), true);

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

    QString shellFile = remotePlugin->createShellFile(config);
    EXPECT_NE(shellFile.length(), 0);
}

#endif
