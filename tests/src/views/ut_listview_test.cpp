// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_listview_test.h"
#include "itemwidget.h"
#include "listview.h"
#include "customcommandpanel.h"
#include "remotemanagementpanel.h"
#include "utils.h"
#include "shortcutmanager.h"
#include "customcommandoptdlg.h"
#include "serverconfigmanager.h"
#include "service.h"
#include "../stub.h"
#include "ut_stub_defines.h"
#include "serverconfigoptdlg.h"


//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QSignalSpy>
#include <QDebug>


UT_ListView_Test::UT_ListView_Test()
{
}

void UT_ListView_Test::SetUp()
{
}

void UT_ListView_Test::TearDown()
{
}

#ifdef UT_LISTVIEW_TEST

TEST_F(UT_ListView_Test, CustomCommandListViewTest)
{
    ListView cmdListWidget(ListType_Custom, nullptr);
    cmdListWidget.resize(242, 600);
    cmdListWidget.show();

    QList<QAction *> cmdActionList;
    const int cmdCount = 10;
    for (int i = 0; i <= cmdCount; i++) {
        QString key = QString(QChar('A' + i));
        QAction *newAction = new QAction;
        newAction->setText(QString("cmd_%1").arg(i));
        newAction->setShortcut(QKeySequence(QString("Ctrl+Shift+%1").arg(key)));
        cmdListWidget.addItem(ItemFuncType_Item, newAction->text(), newAction->shortcut().toString());

        cmdActionList.append(newAction);
        ShortcutManager::instance()->addCustomCommand(*newAction);
    }

    emit Service::instance()->refreshCommandPanel("", "");

    EXPECT_EQ(cmdActionList.isEmpty(), false);

    QAction *firstAction = cmdActionList.first();
    QString firstCmdName = firstAction->text();
    EXPECT_EQ(firstCmdName, QString("cmd_%1").arg(0));

    QAction *lastAction = cmdActionList.last();
    QString lastCmdName = lastAction->text();
    EXPECT_EQ(lastCmdName, QString("cmd_%1").arg(cmdCount));

    cmdListWidget.removeItem(ItemFuncType_Item, firstCmdName);
    EXPECT_EQ(cmdListWidget.count(), cmdActionList.size() - 1);

    QString updateCmdName = Utils::getRandString().toLower();
    cmdListWidget.updateItem(ItemFuncType_Item, lastCmdName, updateCmdName);

    QList<ItemWidget *> itemWidgetList = cmdListWidget.m_itemList;
    ItemWidget *lastItemWidget = itemWidgetList.last();

    cmdListWidget.onCustomItemModify("cmd_0", false);

    cmdListWidget.onCustomItemModify("cmd_0", true);

    cmdListWidget.getWidgetIndex(lastItemWidget);

    //释放内存
    for (int i = 0; i < cmdActionList.size(); i++) {
        QAction *action = cmdActionList.at(i);
        delete action;
    }
}

TEST_F(UT_ListView_Test, onRemoteItemModify)
{
    ListView remoteListWidget(ListType_Custom, nullptr);
    remoteListWidget.resize(242, 600);
    remoteListWidget.show();

    ServerConfigManager *serverConfigManager = ServerConfigManager::instance();
    QList<ServerConfig *> remoteServerList;
    const int remoteCount = 8;
    const QString groupName = "001";
    for (int i = 0; i <= remoteCount; i++) {

        ServerConfig *config = new ServerConfig();
        config->m_serverName = QString("server_%1").arg(i);
        config->m_address = QString("192.168.10.%1").arg(qrand() % 255);
        config->m_userName = QString("%1").arg(Utils::getRandString());
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

        //append item
        int oldCount = remoteListWidget.count();
        remoteListWidget.addItem(ItemFuncType_Item, config->m_serverName, config->m_address);
        EXPECT_TRUE(remoteListWidget.count() == (oldCount + 1));

        remoteServerList.append(config);
    }

    //
    remoteListWidget.onRemoteItemModify("server_0", false);
    EXPECT_TRUE(ServerConfigManager::instance()->m_serverConfigDialogMap.contains("server_0"));

    // 清理不用的数据
    for (ServerConfig *item : remoteServerList) {
        ServerConfigManager::instance()->delServerConfig(item);
        EXPECT_TRUE(ServerConfigManager::instance()->m_serverConfigs[groupName].contains(item) == false);
    }
}

TEST_F(UT_ListView_Test, setFocusFromeIndex)
{
    ListView listWidget(ListType_Custom, nullptr);
    listWidget.resize(242, 600);
    listWidget.show();

    const int count = 11;
    for (int i = 0; i < count; i++) {
        QString key = QString(QChar('A' + i));
        QAction *newAction = new QAction;
        newAction->setText(QString("cmd_%1").arg(i));
        newAction->setShortcut(QKeySequence(QString("Ctrl+Shift+%1").arg(key)));
        listWidget.addItem(ItemFuncType_Item, newAction->text(), newAction->shortcut().toString());

        ShortcutManager::instance()->addCustomCommand(*newAction);
    }
    EXPECT_TRUE(listWidget.count() == count);

    EXPECT_TRUE(ShortcutManager::instance()->m_customCommandActionList.count() >= count);

    listWidget.setFocusFromeIndex(0, ListFocusUp);
    EXPECT_TRUE(listWidget.m_currentIndex == 0);

    listWidget.setFocusFromeIndex(0, ListFocusDown);
    EXPECT_TRUE(listWidget.m_currentIndex == 1);

    listWidget.setFocusFromeIndex(0, ListFocusHome);
    EXPECT_TRUE(listWidget.m_currentIndex == 0);

    listWidget.setFocusFromeIndex(0, ListFocusEnd);
    qDebug() << __LINE__<< listWidget.count() << count;
    EXPECT_TRUE(listWidget.m_currentIndex == (count - 1));
}

//为测试onCustomCommandOptDlgFinished打桩
QAction *stub_getCurCustomCmd()
{
    return ShortcutManager::instance()->getCustomCommandActionList().first();
}

//为测试onCustomCommandOptDlgFinished打桩
void stub_setIsDialogShow(QWidget *parent, bool isDialogShow)
{
    Q_UNUSED(parent)
    Q_UNUSED(isDialogShow)
}

bool stub_isDelCurCommand()
{
    return true;
}

/*******************************************************************************
 1. @函数:    onCustomCommandOptDlgFinished
 2. @作者:    ut000438 王亮
 3. @日期:    2021-02-09
 4. @说明:    onCustomCommandOptDlgFinished单元测试
*******************************************************************************/
TEST_F(UT_ListView_Test, onCustomCommandOptDlgFinished)
{
    ListView listWidget(ListType_Custom, nullptr);

    const int count = 4;
    for (int i = 0; i < count; i++) {
        QString key = QString(QChar('A' + i));
        QAction *newAction = new QAction;
        newAction->setText(QString("cmd_%1").arg(i));
        newAction->setShortcut(QKeySequence(QString("Ctrl+Shift+%1").arg(key)));
        listWidget.addItem(ItemFuncType_Item, newAction->text(), newAction->shortcut().toString());

        ShortcutManager::instance()->addCustomCommand(*newAction);
    }
    EXPECT_TRUE(listWidget.count() == count);

    QAction *firstAction = ShortcutManager::instance()->getCustomCommandActionList().first();
    CustomCommandData itemData;
    itemData.m_cmdName = firstAction->text() + "_modify";
    itemData.m_cmdText = firstAction->data().toString() + "_modify";
    itemData.m_cmdShortcut = firstAction->shortcut().toString() + "_modify";

    Stub s;
    s.set(ADDR(CustomCommandOptDlg, getCurCustomCmd), stub_getCurCustomCmd);
    s.set(ADDR(Service, setIsDialogShow), stub_setIsDialogShow);

    //选中itemData所在行
    listWidget.m_focusState = true;
    listWidget.m_pdlg = new CustomCommandOptDlg(CustomCommandOptDlg::CCT_MODIFY, &itemData, nullptr);
    listWidget.onCustomCommandOptDlgFinished(QDialog::Accepted);
    EXPECT_TRUE(listWidget.currentIndex() == listWidget.indexFromString(itemData.m_cmdName));

    s.set(ADDR(CustomCommandOptDlg, isDelCurCommand), stub_isDelCurCommand);

    int old_index = listWidget.currentIndex();
    listWidget.onCustomCommandOptDlgFinished(QDialog::Rejected);
    EXPECT_TRUE(listWidget.currentIndex() == old_index);


    listWidget.onGroupClicked("/", false);
    EXPECT_TRUE(listWidget.m_focusState == false);
    EXPECT_TRUE(listWidget.currentIndex() == -1);
}

ServerConfig* generateNewServerConfig()
{
    ServerConfig *config = new ServerConfig();
    config->m_serverName = QStringLiteral("new_server_listview_test");
    config->m_address = QStringLiteral("192.168.10.%1").arg(qrand() % 255);
    config->m_userName = QStringLiteral("server_user");
    config->m_password = QStringLiteral("123456");
    config->m_privateKey = QStringLiteral("");
    config->m_port = QStringLiteral("");
    config->m_group = QStringLiteral("new group");
    config->m_path = QStringLiteral("");
    config->m_command = QStringLiteral("");
    config->m_encoding = QStringLiteral("");
    config->m_backspaceKey = QStringLiteral("");
    config->m_deleteKey = QStringLiteral("");

    ServerConfigManager::instance()->saveServerConfig(config);

    ServerConfigManager::instance()->getServerCount(config->m_group);

    ServerConfig *currConfig = ServerConfigManager::instance()->getServerConfig(config->m_serverName);
    return currConfig;
}

//用于onServerConfigOptDlgFinished函数单元测试打桩
void stub_focusCurrentPage_listview()
{
}

//用于onServerConfigOptDlgFinished函数单元测试打桩
bool stub_isDelServer_true()
{
    return true;
}
//用于onServerConfigOptDlgFinished函数单元测试打桩
bool stub_isDelServer_false()
{
    return false;
}

TEST_F(UT_ListView_Test, onServerConfigOptDlgFinished)
{
    ListView listWidget(ListType_Custom, nullptr);
    ServerConfig *curItemServer = generateNewServerConfig();
    QString itemServerName = curItemServer->m_serverName;
    ASSERT_TRUE(curItemServer);
    listWidget.m_configDialog = new ServerConfigOptDlg(ServerConfigOptDlg::SCT_MODIFY, curItemServer, &listWidget);

    {
        //m_configDialog不在m_serverConfigDialogMap里，故不会删除
        Stub stub;
        stub.set(ADDR(MainWindow, focusCurrentPage), stub_focusCurrentPage_listview);
        listWidget.m_focusState == true;
        listWidget.onServerConfigOptDlgFinished(ServerConfigOptDlg::Rejected);
        EXPECT_TRUE(listWidget.currentIndex() == listWidget.indexFromString(curItemServer->m_serverName));
    }

    {
        //m_configDialog不在m_serverConfigDialogMap里，故不会删除
        Stub stub;
        stub.set(ADDR(MainWindow, focusCurrentPage), stub_focusCurrentPage_listview);
        stub.set(ADDR(ServerConfigOptDlg, isDelServer), stub_isDelServer_false);
        listWidget.m_focusState == true;
        listWidget.onServerConfigOptDlgFinished(ServerConfigOptDlg::Accepted);
        EXPECT_TRUE(listWidget.currentIndex() == listWidget.indexFromString(curItemServer->m_serverName));
        //随后listwidget会置空
    }
    {
        Stub stub;
        stub.set(ADDR(MainWindow, focusCurrentPage), stub_focusCurrentPage_listview);
        UT_STUB_QWIDGET_SHOW_APPEND;
        listWidget.m_focusState = true;
        listWidget.m_configDialog->m_bDelOpt = true;
        listWidget.onServerConfigOptDlgFinished(ServerConfigOptDlg::Accepted);
        EXPECT_TRUE(UT_STUB_QWIDGET_SHOW_RESULT);
    }

    EXPECT_TRUE(!ServerConfigManager::instance()->m_serverConfigDialogMap.contains(itemServerName));

    EXPECT_TRUE(listWidget.count() == 0);
}

#endif
