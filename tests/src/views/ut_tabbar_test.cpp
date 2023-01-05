// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_tabbar_test.h"
#include "tabbar.h"
#include "utils.h"
#include "../stub.h"
#include "termwidgetpage.h"
#include "termproperties.h"
#include "ut_stub_defines.h"
#include "windowsmanager.h"

//dtk
#include <DWindowManagerHelper>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QSignalSpy>
#include <QDebug>
#include <QUuid>
#include <QSignalSpy>


UT_Tabbar_Test::UT_Tabbar_Test()
{
}

void UT_Tabbar_Test::SetUp()
{
}

void UT_Tabbar_Test::TearDown()
{
}

QString UT_Tabbar_Test::generateUniqueId()
{
    return QUuid::createUuid().toString();
}

#ifdef UT_TABBAR_TEST
TEST_F(UT_Tabbar_Test, setTabHeight)
{
    TabBar tabbar;
    tabbar.resize(800, 50);
    tabbar.show();

    int tabHeight = 36;
    tabbar.setTabHeight(tabHeight);
    EXPECT_EQ(tabbar.height(), 36);
}

TEST_F(UT_Tabbar_Test, setTabItemMinWidth)
{
    TabBar tabbar;
    tabbar.resize(800, 50);
    tabbar.show();

    int tabItemMinWidth = 110;
    tabbar.setTabItemMinWidth(tabItemMinWidth);
    EXPECT_EQ(tabbar.m_tabItemMinWidth, tabItemMinWidth);

}

TEST_F(UT_Tabbar_Test, setTabItemMaxWidth)
{
    TabBar tabbar;
    tabbar.resize(800, 50);
    tabbar.show();

    int tabItemMaxWidth = 450;
    tabbar.setTabItemMaxWidth(tabItemMaxWidth);
    EXPECT_EQ(tabbar.m_tabItemMaxWidth, tabItemMaxWidth);
}

TEST_F(UT_Tabbar_Test, addTab)
{
    TabBar tabbar;
    tabbar.resize(800, 50);
    tabbar.show();

    QList<QString> tabIdList;
    const int tabCount = 6;
    for (int i = 0; i < tabCount; i++) {
        QString tabName = QString("tab%1").arg(i);
        QString tabIdentifier = generateUniqueId();
        tabbar.addTab(tabIdentifier, tabName);
        tabIdList.append(tabIdentifier);
    }
    EXPECT_EQ(tabbar.count(), tabCount);
}

TEST_F(UT_Tabbar_Test, insertTab)
{
    TabBar tabbar;
    tabbar.resize(800, 50);
    tabbar.show();

    QList<QString> tabIdList;
    const int tabCount = 6;
    for (int i = 0; i < tabCount; i++) {
        QString tabName = QString("tab%1").arg(i);
        QString tabIdentifier = generateUniqueId();
        tabbar.insertTab(i, tabIdentifier, tabName);
        tabIdList.append(tabIdentifier);
    }
    EXPECT_EQ(tabbar.count(), tabCount);
}

TEST_F(UT_Tabbar_Test, removeTab)
{
    TabBar tabbar;
    tabbar.resize(800, 50);
    tabbar.show();

    QObject::disconnect(&tabbar, &DTabBar::tabIsRemoved, nullptr, nullptr);

    QList<QString> tabIdList;
    const int tabCount = 6;
    for (int i = 0; i < tabCount; i++) {
        QString tabName = QString("tab%1").arg(i);
        QString tabIdentifier = generateUniqueId();
        tabbar.insertTab(i, tabIdentifier, tabName);
        tabIdList.append(tabIdentifier);
    }
    EXPECT_EQ(tabbar.count(), tabCount);

    QString firstTabId = tabbar.identifier(0);
    tabbar.removeTab(firstTabId);

    EXPECT_EQ(tabbar.count(), tabCount - 1);
}


TEST_F(UT_Tabbar_Test, identifier)
{
    TabBar tabbar;
    tabbar.resize(800, 50);
    tabbar.show();

    QList<QString> tabIdList;
    const int tabCount = 6;
    for (int i = 0; i < tabCount; i++) {
        QString tabName = QString("tab%1").arg(i);
        QString tabIdentifier = generateUniqueId();
        tabbar.addTab(tabIdentifier, tabName);
        tabIdList.append(tabIdentifier);
    }

    int firstTabIndex = 0;
    EXPECT_EQ(tabbar.identifier(firstTabIndex), tabIdList.first());
}

TEST_F(UT_Tabbar_Test, getIndexByIdentifier)
{
    TabBar tabbar;
    tabbar.resize(800, 50);
    tabbar.show();

    QList<QString> tabIdList;
    const int tabCount = 6;
    for (int i = 0; i < tabCount; i++) {
        QString tabName = QString("tab%1").arg(i);
        QString tabIdentifier = generateUniqueId();
        tabbar.addTab(tabIdentifier, tabName);
        tabIdList.append(tabIdentifier);
    }

    int firstTabIndex = 0;
    int tabIndex = tabbar.getIndexByIdentifier(tabIdList.first());
    EXPECT_EQ(tabIndex, firstTabIndex);
}

TEST_F(UT_Tabbar_Test, setTabText)
{
    TabBar tabbar;
    tabbar.resize(800, 50);
    tabbar.show();

    QList<QString> tabIdList;
    const int tabCount = 6;
    for (int i = 0; i < tabCount; i++) {
        QString tabName = QString("tab%1").arg(i);
        QString tabIdentifier = generateUniqueId();
        tabbar.addTab(tabIdentifier, tabName);
        tabIdList.append(tabIdentifier);
    }

    int firstTabIndex = 0;
    QString tabText = QString("tab001");
    tabbar.setTabText(tabIdList.first(), tabText);
    EXPECT_EQ(tabbar.tabText(firstTabIndex), tabText);
}

TEST_F(UT_Tabbar_Test, getSessionIdTabIndexMap)
{
    TabBar tabbar;
    tabbar.resize(800, 50);
    tabbar.show();

    QMap<int, int> sessionIdTabIndexMap = tabbar.getSessionIdTabIndexMap();
    EXPECT_EQ(sessionIdTabIndexMap.isEmpty(), true);
}

TEST_F(UT_Tabbar_Test, saveSessionIdWithTabIndex)
{
    TabBar tabbar;
    tabbar.resize(800, 50);
    tabbar.show();

    const int mapSize = 5;
    for (int i = 0; i < mapSize; i++) {
        tabbar.saveSessionIdWithTabIndex(i + 1, i);
    }

    QMap<int, int> sessionIdTabIndexMap = tabbar.getSessionIdTabIndexMap();
    EXPECT_EQ((mapSize == sessionIdTabIndexMap.size()), true);
}

TEST_F(UT_Tabbar_Test, saveSessionIdWithTabId)
{
    TabBar tabbar;
    tabbar.resize(800, 50);
    tabbar.show();

    const int mapSize = 5;
    for (int i = 0; i < mapSize; i++) {
        tabbar.saveSessionIdWithTabId(i, generateUniqueId());
    }

    QMap<int, QString> sessionIdTabIdMap = tabbar.m_sessionIdTabIdMap;
    EXPECT_EQ((mapSize == sessionIdTabIdMap.size()), true);
}

TEST_F(UT_Tabbar_Test, queryIndexBySessionId)
{
    TabBar tabbar;
    tabbar.resize(800, 50);
    tabbar.show();

    const int mapSize = 5;
    for (int i = 0; i < mapSize; i++) {
        tabbar.saveSessionIdWithTabIndex(i + 1, i);
    }

    int queryIndex = tabbar.queryIndexBySessionId(0);
    EXPECT_EQ((queryIndex == -1), true);
}

TEST_F(UT_Tabbar_Test, setChangeTextColor)
{
    TabBar tabbar;
    tabbar.resize(800, 50);
    tabbar.show();

    QList<QString> tabIdList;
    const int tabCount = 6;
    for (int i = 0; i < tabCount; i++) {
        QString tabName = QString("tab%1").arg(i);
        QString tabIdentifier = generateUniqueId();
        tabbar.addTab(tabIdentifier, tabName);
        tabIdList.append(tabIdentifier);
    }
    EXPECT_TRUE(tabIdList.count() == tabCount);

    QString firstTabId = tabbar.identifier(0);
    tabbar.setChangeTextColor(firstTabId);
    EXPECT_TRUE(tabbar.m_tabStatusMap.contains(firstTabId));
}

TEST_F(UT_Tabbar_Test, setNeedChangeTextColor)
{
    TabBar tabbar;
    tabbar.resize(800, 50);
    tabbar.show();

    QList<QString> tabIdList;
    const int tabCount = 6;
    for (int i = 0; i < tabCount; i++) {
        QString tabName = QString("tab%1").arg(i);
        QString tabIdentifier = generateUniqueId();
        tabbar.addTab(tabIdentifier, tabName);
        tabIdList.append(tabIdentifier);
    }
    EXPECT_TRUE(tabIdList.count() == tabCount);

    QColor redColor = QColor(Qt::red);
    QString firstTabId = tabbar.identifier(0);
    tabbar.setNeedChangeTextColor(firstTabId, redColor);
    EXPECT_TRUE(tabbar.isNeedChangeTextColor(firstTabId));
    EXPECT_TRUE(tabbar.m_tabChangedTextColor == redColor);
}

TEST_F(UT_Tabbar_Test, removeNeedChangeTextColor)
{
    TabBar tabbar;
    tabbar.resize(800, 50);
    tabbar.show();

    QList<QString> tabIdList;
    const int tabCount = 6;
    for (int i = 0; i < tabCount; i++) {
        QString tabName = QString("tab%1").arg(i);
        QString tabIdentifier = generateUniqueId();
        tabbar.addTab(tabIdentifier, tabName);
        tabIdList.append(tabIdentifier);
    }
    EXPECT_TRUE(tabIdList.count() == tabCount);

    QString firstTabId = tabbar.identifier(0);
    tabbar.setChangeTextColor(firstTabId);
    EXPECT_TRUE(tabbar.m_tabStatusMap.contains(firstTabId));

    tabbar.removeNeedChangeTextColor(firstTabId);
    EXPECT_TRUE(tabbar.m_tabStatusMap.contains(firstTabId) == false);
}

TEST_F(UT_Tabbar_Test, isNeedChangeTextColor)
{
    TabBar tabbar;
    tabbar.resize(800, 50);
    tabbar.show();

    QList<QString> tabIdList;
    const int tabCount = 6;
    for (int i = 0; i < tabCount; i++) {
        QString tabName = QString("tab%1").arg(i);
        QString tabIdentifier = generateUniqueId();
        tabbar.addTab(tabIdentifier, tabName);
        tabIdList.append(tabIdentifier);
    }
    EXPECT_TRUE(tabIdList.count() == tabCount);

    QColor redColor = QColor(Qt::red);
    QString firstTabId = tabbar.identifier(0);
    tabbar.setNeedChangeTextColor(firstTabId, redColor);
    EXPECT_TRUE(tabbar.isNeedChangeTextColor(firstTabId));
    EXPECT_TRUE(tabbar.m_tabChangedTextColor == redColor);
}

TEST_F(UT_Tabbar_Test, setClearTabColor)
{
    TabBar tabbar;
    tabbar.resize(800, 50);
    tabbar.show();

    QList<QString> tabIdList;
    const int tabCount = 6;
    for (int i = 0; i < tabCount; i++) {
        QString tabName = QString("tab%1").arg(i);
        QString tabIdentifier = generateUniqueId();
        tabbar.addTab(tabIdentifier, tabName);
        tabIdList.append(tabIdentifier);
    }
    EXPECT_TRUE(tabIdList.count() == tabCount);

    QString firstTabId = tabbar.identifier(0);
    tabbar.setChangeTextColor(firstTabId);
    EXPECT_TRUE(tabbar.m_tabStatusMap.value(firstTabId) == TabTextColorStatus_Changed);

    tabbar.setClearTabColor(firstTabId);
    EXPECT_TRUE(tabbar.m_tabStatusMap.value(firstTabId) == TabTextColorStatus_Default);
}

//用于createDragPixmapFromTab测试打桩
TermWidgetPage *stub_getTermPage(const QString &identifier)
{
    Q_UNUSED(identifier)

    TermProperties properties;
    TermWidgetPage *widgetPage = new TermWidgetPage(properties, nullptr);
    widgetPage->resize(800, 600);
    return widgetPage;
}

//用于createDragPixmapFromTab测试打桩
bool stub_hasComposite()
{
    return true;
}

/*******************************************************************************
 1. @函数:    Tabbar类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-23
 4. @说明:    createDragPixmapFromTab单元测试
*******************************************************************************/
TEST_F(UT_Tabbar_Test, createDragPixmapFromTab)
{
    Stub s;
    s.set(ADDR(MainWindow, getTermPage), stub_getTermPage);

    NormalWindow w(TermProperties("/"));
    TabBar tabbar(&w);
    QString tabName = QString("tab01");
    QString tabIdentifier = generateUniqueId();
    tabbar.addTab(tabIdentifier, tabName);
//    EXPECT_TRUE(tabbar.m_tabIdentifierList.contains(tabIdentifier));

    QStyleOptionTab styleOptionTab;
    QPoint hotSpot = QPoint(1, 1);
    EXPECT_TRUE(tabbar.createDragPixmapFromTab(0, styleOptionTab, &hotSpot).isNull() == false);

    s.set(ADDR(DWindowManagerHelper, hasComposite), stub_hasComposite);
    EXPECT_TRUE(tabbar.createDragPixmapFromTab(0, styleOptionTab, &hotSpot).isNull() == false);
}

static QWindow *ut_tablbar_dragIconWindow()
{
    static QWindow w;
    return &w;
}

static bool ut_window_setProperty(const char *, const QVariant &)
{
    return true;
}

static QAction *stub_exec(const QPoint &, QAction *)
{
    return nullptr;
}


TEST_F(UT_Tabbar_Test, handleDragActionChanged)
{
    TabBar tabbar;
    tabbar.resize(800, 50);
    tabbar.show();

    Stub stub;
    stub.set(ADDR(DTabBar, dragIconWindow), ut_tablbar_dragIconWindow);
    stub.set(ADDR(QWindow, setProperty), ut_window_setProperty);

    qApp->setOverrideCursor(Qt::CrossCursor);
    tabbar.handleDragActionChanged(Qt::IgnoreAction);
    EXPECT_TRUE(qApp->overrideCursor()->shape() == Qt::ArrowCursor);

    tabbar.handleDragActionChanged(Qt::MoveAction);
    qDebug() << qApp->overrideCursor();
    EXPECT_TRUE(qApp->overrideCursor());
}

TEST_F(UT_Tabbar_Test, handleMiddleButtonClick)
{
    NormalWindow *w = new NormalWindow(TermProperties("/"));
    TabBar *tabbar = new TabBar(w);
    tabbar->addTab(generateUniqueId(), "tab01");
    QSignalSpy spy(tabbar, &DTabBar::tabCloseRequested);
    QMouseEvent mouseEvent(QEvent::MouseButtonRelease, tabbar->tabRect(0).center(), Qt::MidButton, Qt::NoButton, Qt::NoModifier);
    tabbar->handleMiddleButtonClick(&mouseEvent);
    qDebug() << spy.count();
    EXPECT_TRUE(1 == spy.count());

    w->deleteLater();
}


TEST_F(UT_Tabbar_Test, handleRightButtonClick)
{
    Stub stub;
    stub.set((QAction * (QMenu::*)(const QPoint &, QAction *)) ADDR(QMenu, exec), stub_exec);

    NormalWindow *w = new NormalWindow(TermProperties("/"));
    TabBar *tabbar = w->m_tabbar;
    w->addTab(TermProperties("/"));
    QMouseEvent mouseEvent(QEvent::MouseButtonRelease, tabbar->tabRect(0).center(), Qt::RightButton, Qt::NoButton, Qt::NoModifier);
    EXPECT_TRUE(nullptr == tabbar->m_closeTabAction);
    tabbar->handleRightButtonClick(&mouseEvent);
    //首次点击时，会初始化m_closeTabAction
    EXPECT_TRUE(tabbar->m_closeTabAction);

    w->deleteLater();
}

TEST_F(UT_Tabbar_Test, onCloseOtherTabActionTriggered)
{
    NormalWindow *w = new NormalWindow(TermProperties("/"));
    TabBar *tabbar = w->m_tabbar;
    w->addTab(TermProperties("/"));
    w->addTab(TermProperties("/"));
    w->addTab(TermProperties("/"));

    tabbar->m_rightClickTab = 0;
    EXPECT_TRUE(4 == tabbar->count());
    tabbar->onCloseOtherTabActionTriggered();
    EXPECT_TRUE(1 == tabbar->count());
    qDebug() << tabbar->count();

    tabbar->m_rightClickTab = 0;
    QSignalSpy spy(tabbar, &TabBar::showRenameTabDialog);
    tabbar->onRenameTabActionTriggered();
    qDebug() << spy.count();
    EXPECT_TRUE(1 == spy.count());


    w->deleteLater();
}

TEST_F(UT_Tabbar_Test, handleTabMoved)
{
    NormalWindow *w = new NormalWindow(TermProperties("/"));
    TabBar *tabbar = w->m_tabbar;
    w->addTab(TermProperties("/"));
    w->addTab(TermProperties("/"));
    w->addTab(TermProperties("/"));
    w->addTab(TermProperties("/"));

    //只是交换TabBar中的保存的list，非DTabBar
    QString id1 = tabbar->m_tabIdentifierList.value(1);
    QString id2 = tabbar->m_tabIdentifierList.value(2);
    tabbar->handleTabMoved(1, 2);
    EXPECT_TRUE(tabbar->m_tabIdentifierList.value(1) == id2);
    EXPECT_TRUE(tabbar->m_tabIdentifierList.value(2) == id1);

    //释放其中一个termwidget
    EXPECT_TRUE(w->m_termStackWidget->count() == 5);
    tabbar->handleTabReleased(0);
    EXPECT_TRUE(w->m_termStackWidget->count() == 4);

    w->deleteLater();
}

TEST_F(UT_Tabbar_Test, createWindowFromTermPage)
{
    NormalWindow *w = new NormalWindow(TermProperties("/"));
    TabBar *tabbar = w->m_tabbar;
    w->addTab(TermProperties("/"));
    w->addTab(TermProperties("/"));
    w->addTab(TermProperties("/"));
    w->addTab(TermProperties("/"));

    int oldcount = WindowsManager::instance()->getNormalWindowList().count();
    QString tab1 = tabbar->tabText(1);
    tabbar->createWindowFromTermPage(tab1, w->currentPage(), true);
    int curcount = WindowsManager::instance()->getNormalWindowList().count();
    EXPECT_TRUE(curcount == (oldcount + 1));

    w->deleteLater();
}


#endif
