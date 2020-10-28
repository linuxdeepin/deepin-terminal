#include "ut_tabbar_test.h"

#define private public
#include "tabbar.h"
#undef private
#include "utils.h"

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QSignalSpy>
#include <QDebug>

UT_Tabbar_Test::UT_Tabbar_Test()
{
}

void UT_Tabbar_Test::SetUp()
{
}

void UT_Tabbar_Test::TearDown()
{
}

#ifdef UT_TABBAR_TEST
TEST_F(UT_Tabbar_Test, TabbarTest)
{
    TabBar tabbar;
    tabbar.resize(800, 50);
    tabbar.show();
    EXPECT_EQ(tabbar.isVisible(), true);

    int tabHeight = 36;
    tabbar.setTabHeight(tabHeight);
    EXPECT_EQ(tabbar.height(), 36);

    int tabItemMinWidth = 110;
    tabbar.setTabItemMinWidth(tabItemMinWidth);
    EXPECT_EQ(tabbar.m_tabItemMinWidth, tabItemMinWidth);

    int tabItemMaxWidth = 450;
    tabbar.setTabItemMaxWidth(tabItemMaxWidth);
    EXPECT_EQ(tabbar.m_tabItemMaxWidth, tabItemMaxWidth);

    QList<QString> tabIdList;
    const int tabCount = 6;
    for(int i=0; i<tabCount; i++)
    {
        QString tabName = QString("tab%1").arg(i);
        QString tabIdentifier = Utils::getRandString().toLower();
        tabbar.addTab(tabIdentifier, tabName);
        tabIdList.append(tabIdentifier);
#ifdef ENABLE_UI_TEST
        QTest::qWait(100);
#endif
    }
    EXPECT_EQ(tabbar.count(), tabCount);

    int firstTabIndex = 0;
    EXPECT_EQ(tabbar.identifier(firstTabIndex), tabIdList.first());

    int tabIndex = tabbar.getIndexByIdentifier(tabIdList.first());
    EXPECT_EQ(tabIndex, firstTabIndex);

    QString tabText = QString("tab001");
    tabbar.setTabText(tabIdList.first(), tabText);
    EXPECT_EQ(tabbar.tabText(firstTabIndex), tabText);

    tabbar.removeTab(tabIdList.first());
    EXPECT_EQ(tabbar.count(), tabCount-1);

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
}
#endif
