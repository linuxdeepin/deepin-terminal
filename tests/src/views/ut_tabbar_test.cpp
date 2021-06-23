/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:     wangliang <wangliang@uniontech.com>
 *
 * Maintainer: wangliang <wangliang@uniontech.com>
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

#include "ut_tabbar_test.h"

#include "tabbar.h"
#include "utils.h"
#include "../stub.h"
#include "termwidgetpage.h"

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QSignalSpy>
#include <QDebug>
#include <QUuid>

#include <DWindowManagerHelper>

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

    QString firstTabId = tabbar.identifier(0);
    tabbar.setChangeTextColor(firstTabId);
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

    QColor redColor = QColor(Qt::red);

    QString firstTabId = tabbar.identifier(0);
    tabbar.setNeedChangeTextColor(firstTabId, redColor);
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

    QString firstTabId = tabbar.identifier(0);
    tabbar.setChangeTextColor(firstTabId);

    tabbar.removeNeedChangeTextColor(firstTabId);
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

    QColor redColor = QColor(Qt::red);

    QString firstTabId = tabbar.identifier(0);
    tabbar.setNeedChangeTextColor(firstTabId, redColor);

    EXPECT_EQ(tabbar.isNeedChangeTextColor(firstTabId), true);
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

    EXPECT_EQ(tabIdList.size(), tabCount);

    QString firstTabId = tabbar.identifier(0);
    tabbar.setChangeTextColor(firstTabId);

    tabbar.setClearTabColor(firstTabId);
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

    TabBar tabbar;
    QString tabName = QString("tab01");
    QString tabIdentifier = generateUniqueId();
    tabbar.addTab(tabIdentifier, tabName);

    QStyleOptionTab styleOptionTab;
    QPoint hotSpot = QPoint(1, 1);
    tabbar.createDragPixmapFromTab(0, styleOptionTab, &hotSpot);

    s.set(ADDR(DWindowManagerHelper, hasComposite), stub_hasComposite);

    tabbar.createDragPixmapFromTab(0, styleOptionTab, &hotSpot);

    s.reset(ADDR(MainWindow, getTermPage));
    s.reset(ADDR(DWindowManagerHelper, hasComposite));
}

TEST_F(UT_Tabbar_Test, handleDragActionChanged)
{
    TabBar tabbar;
    tabbar.resize(800, 50);
    tabbar.show();

    tabbar.handleDragActionChanged(Qt::MoveAction);

    tabbar.handleDragActionChanged(Qt::IgnoreAction);
}

#endif
