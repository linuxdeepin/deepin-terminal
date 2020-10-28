#include "ut_titlebar_test.h"

#define private public
#include "titlebar.h"
#include "tabbar.h"
#include "mainwindow.h"
#include "service.h"
#undef private

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QSignalSpy>
#include <QDebug>
#include <QMainWindow>
#include <DTitlebar>

DWIDGET_USE_NAMESPACE

UT_TitleBar_Test::UT_TitleBar_Test()
{
}

void UT_TitleBar_Test::SetUp()
{
    if (!Service::instance()->property("isServiceInit").toBool())
    {
        Service::instance()->init();
        Service::instance()->setProperty("isServiceInit", true);
    }

    m_normalTermProperty[QuakeMode] = false;
    m_normalTermProperty[SingleFlag] = true;
    m_normalWindow = new NormalWindow(m_normalTermProperty, nullptr);
}

void UT_TitleBar_Test::TearDown()
{
    delete m_normalWindow;
}

#ifdef UT_TITLEBAR_TEST
TEST_F(UT_TitleBar_Test, TitleBarTest)
{
    m_normalWindow->resize(800, 600);
    m_normalWindow->show();
    EXPECT_EQ(m_normalWindow->isVisible(), true);

    DTitlebar *dtkTitleBar = m_normalWindow->titlebar();
    TitleBar *titleBar = qobject_cast<TitleBar*>(dtkTitleBar->customWidget());
    EXPECT_NE(titleBar, nullptr);
    EXPECT_EQ(titleBar->isVisible(), true);

    //清空titleBar布局
    QHBoxLayout *titleBarLayout = titleBar->m_layout;
    QLayoutItem *child;
    while ((child = titleBarLayout->takeAt(0)) != nullptr)
    {
        if(child->widget())
        {
            child->widget()->setParent(nullptr);
        }
        delete child;
    }

    TabBar tabbar;
    tabbar.resize(800, 50);
    tabbar.show();
    EXPECT_EQ(tabbar.isVisible(), true);

    int tabHeight = 36;
    tabbar.setTabHeight(tabHeight);
    EXPECT_EQ(tabbar.height(), 36);

    QString tabName = QString("TestTitleBar");
    QString tabIdentifier = Utils::getRandString().toLower();
    tabbar.addTab(tabIdentifier, tabName);
    titleBar->setTabBar(&tabbar);

    int rightSpace = titleBar->rightSpace();
    EXPECT_EQ(rightSpace, titleBar->m_rightSpace);

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
}
#endif
