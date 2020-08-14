#include "ut_listview_test.h"

#include "listview.h"
#include "customcommandpanel.h"

#include <QString>
#include <QDebug>
#include <QtGui>

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>


UT_ListView_Test::UT_ListView_Test()
{
}

void UT_ListView_Test::SetUp()
{
}

void UT_ListView_Test::TearDown()
{
}

TEST_F(UT_ListView_Test, CustomCommandListViewTest)
{
    const int PANEL_WIDTH = 242;
    const int PANEL_HEIGHT = 600;
    CustomCommandPanel panel;
    panel.resize(PANEL_WIDTH, PANEL_HEIGHT);
    panel.show();
    EXPECT_EQ(panel.size().width(), PANEL_WIDTH);
    EXPECT_EQ(panel.size().height(), PANEL_HEIGHT);

    panel.refreshCmdPanel();

    ListView cmdListWidget(ListType_Custom, &panel);

    QList<QAction *> cmdActionList;
    for(int i=0; i<=10; i++)
    {
        QString key = QString(QChar('A'+i));
        QAction *newAction = new QAction;
        newAction->setText(QString("cmd_%1").arg(i));
        newAction->setShortcut(QKeySequence(QString("Ctrl+Shift+%1").arg(key)));
        cmdListWidget.addItem(ItemFuncType_Item, newAction->text(), newAction->shortcut().toString());

        cmdActionList.append(newAction);
    }

    QObject::connect(&cmdListWidget, &ListView::itemClicked, &panel, &CustomCommandPanel::doCustomCommand);
    QObject::connect(&cmdListWidget, &ListView::listItemCountChange, &panel, &CustomCommandPanel::refreshCmdSearchState);
    QObject::connect(&cmdListWidget, &ListView::focusOut, &panel, &CustomCommandPanel::onFocusOut);

    ASSERT_EQ(cmdActionList.isEmpty(), false);

    QAction *firstAction = cmdActionList.first();
    QString cmdName = firstAction->text();
    EXPECT_EQ(cmdName, QString("cmd_%1").arg(0));
    qDebug() << cmdName << endl;

    QAction *lastAction = cmdActionList.last();
    cmdName = lastAction->text();
    EXPECT_EQ(cmdName, QString("cmd_%1").arg(10));
    qDebug() << cmdName << endl;

    for(int i=0; i<cmdActionList.size(); i++)
    {
        QAction *action = cmdActionList.at(i);
        delete action;
    }
}


