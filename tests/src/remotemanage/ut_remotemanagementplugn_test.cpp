// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_remotemanagementplugn_test.h"

#include "remotemanagementplugn.h"
#include "mainwindow.h"
#include "service.h"

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QFile>

class ServerConfigClass : public QObject{
public:
    explicit ServerConfigClass(QObject *parent);
    virtual ~ServerConfigClass();
    ServerConfig config;
};

ServerConfigClass::ServerConfigClass(QObject *parent) : QObject(parent)
{

}

ServerConfigClass::~ServerConfigClass()
{

}

UT_RemoteManagementPlugin_Test::UT_RemoteManagementPlugin_Test()
{
}

void UT_RemoteManagementPlugin_Test::SetUp()
{
    if (!Service::instance()->property("isServiceInit").toBool()) {
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
}

TEST_F(UT_RemoteManagementPlugin_Test, setBackspaceKey)
{
    EXPECT_EQ(m_normalWindow->isVisible(), true);

    m_normalWindow->showPlugin(MainWindow::PLUGIN_TYPE_REMOTEMANAGEMENT);

    RemoteManagementPlugin *remotePlugin = m_normalWindow->findChild<RemoteManagementPlugin *>();
    RemoteManagementTopPanel *remoteTopPanel = remotePlugin->getRemoteManagementTopPanel();
    EXPECT_NE(remoteTopPanel, nullptr);
    EXPECT_EQ(remoteTopPanel->isVisible(), true);

    TermWidget *termWidget = m_normalWindow->currentActivatedTerminal();
    remotePlugin->setBackspaceKey(termWidget, QString("control-h"));
    remotePlugin->setBackspaceKey(termWidget, QString("auto"));
    remotePlugin->setBackspaceKey(termWidget, QString("escape-sequence"));
    remotePlugin->setBackspaceKey(termWidget, QString("ascii-del"));
    remotePlugin->setBackspaceKey(termWidget, QString("tty"));
}

TEST_F(UT_RemoteManagementPlugin_Test, setDeleteKey)
{
    EXPECT_EQ(m_normalWindow->isVisible(), true);

    m_normalWindow->showPlugin(MainWindow::PLUGIN_TYPE_REMOTEMANAGEMENT);

    RemoteManagementPlugin *remotePlugin = m_normalWindow->findChild<RemoteManagementPlugin *>();
    RemoteManagementTopPanel *remoteTopPanel = remotePlugin->getRemoteManagementTopPanel();
    EXPECT_NE(remoteTopPanel, nullptr);
    EXPECT_EQ(remoteTopPanel->isVisible(), true);

    TermWidget *termWidget = m_normalWindow->currentActivatedTerminal();
    remotePlugin->setDeleteKey(termWidget, QString("control-h"));
    remotePlugin->setDeleteKey(termWidget, QString("auto"));
    remotePlugin->setDeleteKey(termWidget, QString("escape-sequence"));
    remotePlugin->setDeleteKey(termWidget, QString("ascii-del"));
    remotePlugin->setDeleteKey(termWidget, QString("tty"));
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
    delete config;
}

/*******************************************************************************
 1. @函数:    createShellFile
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-13
 4. @说明:    测试创建脚本的各种情况
*******************************************************************************/
TEST_F(UT_RemoteManagementPlugin_Test, createShellFile2)
{
    RemoteManagementPlugin *pRemotePlugin = new RemoteManagementPlugin;
    ServerConfig config;
    config.m_userName = "root";
    config.m_address = "127.0.0.1";
    config.m_port = "22";
    // 密钥为空
    config.m_privateKey = "";
    config.m_password = "1";
    config.m_path = "/";
    config.m_command = "ls";
    // 生成配置文件执行位置及其参数
    QString command = pRemotePlugin->createShellFile(&config);
    QStringList strList = command.split(" ");
    QFile file(strList.first());
    // 文件存在
    EXPECT_EQ(file.exists(), true);

    // 密钥不为空
    config.m_privateKey = "/";
    command = pRemotePlugin->createShellFile(&config);
    strList.clear();
    strList = command.split(" ");
    file.setFileName(strList.first());
    // 文件存在
    EXPECT_EQ(file.exists(), true);
    // 删除文件
    file.remove();

    delete pRemotePlugin;
}

TEST_F(UT_RemoteManagementPlugin_Test, doCennectServer)
{
    NormalWindow *normalWindow = new NormalWindow(TermProperties({{WorkingDir, "/"},{QuakeMode, false},{SingleFlag, true}}), nullptr);
    ServerConfigClass *config = new ServerConfigClass(normalWindow);
    RemoteManagementPlugin *pRemotePlugin = new RemoteManagementPlugin(normalWindow);

    config->config.m_userName = "root";
    config->config.m_address = "127.0.0.1";
    config->config.m_port = "22";
    // 密钥为空
    config->config.m_privateKey = "";
    config->config.m_password = "1";
    config->config.m_path = "/";
    config->config.m_command = "ls";
    pRemotePlugin->initPlugin(normalWindow);
    pRemotePlugin->doCennectServer(&config->config);
    //代码中有singleShot(100)的调用
    QTest::qWait(1000);
    //连接请求发出后，会showPlugin
    EXPECT_TRUE(normalWindow->m_CurrentShowPlugin == MainWindow::PLUGIN_TYPE_NONE);
    normalWindow->deleteLater();
}

#endif
