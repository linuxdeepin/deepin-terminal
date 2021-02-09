#include "ut_listview_test.h"

#include "itemwidget.h"
#include "listview.h"
#include "customcommandpanel.h"
#include "remotemanagementpanel.h"
#include "utils.h"
#include "shortcutmanager.h"
#include "customcommandoptdlg.h"
#include "service.h"
#include "stub.h"

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
    EXPECT_EQ(lastItemWidget->m_firstText, updateCmdName);

    cmdListWidget.onCustomItemModify("cmd_0", false);

    cmdListWidget.onCustomItemModify("cmd_0", true);

    int index = cmdListWidget.getWidgetIndex(lastItemWidget);
    EXPECT_GE(index, 0);

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
    for (int i = 0; i <= remoteCount; i++) {

        ServerConfig *config = new ServerConfig();
        config->m_serverName = QString("server_%1").arg(i);
        config->m_address = QString("192.168.10.%1").arg(qrand() % 255);
        config->m_userName = QString("%1").arg(Utils::getRandString());
        config->m_password = QString("123");
        config->m_privateKey = QString("");
        config->m_port = QString("");
        config->m_group = QString("");
        config->m_path = QString("");
        config->m_command = QString("");
        config->m_encoding = QString("");
        config->m_backspaceKey = QString("");
        config->m_deleteKey = QString("");

        serverConfigManager->saveServerConfig(config);

        remoteListWidget.addItem(ItemFuncType_Item, config->m_serverName, config->m_address);

        remoteServerList.append(config);
    }

    remoteListWidget.onRemoteItemModify("server_0", false);

    // 清理不用的数据
    for (ServerConfig *item : remoteServerList) {
        if (item->m_serverName.contains("server_")) {
            remoteServerList.removeOne(item);
            qDebug() << "####### delete " << item->m_serverName;
            // 删除测试数据
            ServerConfigManager::instance()->delServerConfig(item);
        }
    }
}

TEST_F(UT_ListView_Test, setFocusFromeIndex)
{
    ListView listWidget(ListType_Custom, nullptr);
    listWidget.resize(242, 600);
    listWidget.show();

    const int count = 10;
    for (int i = 0; i <= count; i++) {
        QString key = QString(QChar('A' + i));
        QAction *newAction = new QAction;
        newAction->setText(QString("cmd_%1").arg(i));
        newAction->setShortcut(QKeySequence(QString("Ctrl+Shift+%1").arg(key)));
        listWidget.addItem(ItemFuncType_Item, newAction->text(), newAction->shortcut().toString());

        ShortcutManager::instance()->addCustomCommand(*newAction);
    }

    listWidget.setFocusFromeIndex(0, true);

    listWidget.setFocusFromeIndex(0, false);
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

    const int count = 3;
    for (int i = 0; i <= count; i++) {
        QString key = QString(QChar('A' + i));
        QAction *newAction = new QAction;
        newAction->setText(QString("cmd_%1").arg(i));
        newAction->setShortcut(QKeySequence(QString("Ctrl+Shift+%1").arg(key)));
        listWidget.addItem(ItemFuncType_Item, newAction->text(), newAction->shortcut().toString());

        ShortcutManager::instance()->addCustomCommand(*newAction);
    }

    QAction *firstAction = ShortcutManager::instance()->getCustomCommandActionList().first();
    CustomCommandData itemData;
    itemData.m_cmdName = firstAction->text() + "_modify";
    itemData.m_cmdText = firstAction->data().toString() + "_modify";
    itemData.m_cmdShortcut = firstAction->shortcut().toString() + "_modify";

    Stub s;
    s.set(ADDR(CustomCommandOptDlg, getCurCustomCmd), stub_getCurCustomCmd);
    s.set(ADDR(Service, setIsDialogShow), stub_setIsDialogShow);

    listWidget.m_pdlg = new CustomCommandOptDlg(CustomCommandOptDlg::CCT_MODIFY, &itemData, nullptr);
    listWidget.onCustomCommandOptDlgFinished(QDialog::Accepted);

    s.set(ADDR(CustomCommandOptDlg, isDelCurCommand), stub_isDelCurCommand);

    listWidget.onCustomCommandOptDlgFinished(QDialog::Rejected);

    // 打桩还原
    s.reset(ADDR(CustomCommandOptDlg, isDelCurCommand));

    s.reset(ADDR(CustomCommandOptDlg, getCurCustomCmd));
    s.reset(ADDR(Service, setIsDialogShow));
}

#endif
