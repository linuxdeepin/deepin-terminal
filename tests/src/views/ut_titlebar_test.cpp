// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_titlebar_test.h"

#include "titlebar.h"
#include "tabbar.h"
#include "mainwindow.h"
#include "service.h"
#include "../stub.h"

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
    if (!Service::instance()->property("isServiceInit").toBool()) {
        Service::instance()->setProperty("isServiceInit", true);
    }

    m_normalTermProperty[QuakeMode] = false;
    m_normalTermProperty[SingleFlag] = true;
    m_normalWindow = new NormalWindow(m_normalTermProperty, nullptr);
}

UT_TitleBar_Test::~UT_TitleBar_Test()
{
    delete m_normalWindow;
}

#ifdef UT_TITLEBAR_TEST

bool ut_isDXcbPlatform()
{
    return true;
}

TEST_F(UT_TitleBar_Test, TitleBarTest)
{
    Stub stub;
    stub.set(ADDR(DApplication,isDXcbPlatform),ut_isDXcbPlatform);
    TitleBar *bar = new TitleBar(nullptr);
    delete bar;
    m_normalWindow->resize(800, 600);
    m_normalWindow->show();
    EXPECT_EQ(m_normalWindow->isVisible(), true);

    DTitlebar *dtkTitleBar = m_normalWindow->titlebar();
    TitleBar *titleBar = qobject_cast<TitleBar *>(dtkTitleBar->customWidget());
    EXPECT_NE(titleBar, nullptr);
    EXPECT_EQ(titleBar->isVisible(), true);

    //清空titleBar布局
    QHBoxLayout *titleBarLayout = titleBar->m_layout;
    QLayoutItem *child;
    while ((child = titleBarLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
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
}

/*******************************************************************************
 1. @函数:    mousePressEvent / mouseMoveEvent
 2. @说明:    覆盖 TitleBar 鼠标事件处理分支（垂直 resize 相关）
*******************************************************************************/
TEST_F(UT_TitleBar_Test, MouseEvents)
{
    TitleBar bar;
    bar.resize(800, 36);
    bar.setVerResized(true);
    bar.show();
    EXPECT_EQ(bar.isVisible(), true);

    // 模拟鼠标按下：覆盖 mousePressEvent 中 window 分支
    QMouseEvent mousePress(QEvent::MouseButtonPress, QPointF(100, 30),
                           Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(&bar, &mousePress);

    // 鼠标移动且未按键，靠近底部 => 设置 SizeVerCursor
    bar.setMouseTracking(true);
    QMouseEvent mouseMoveNoBtn(QEvent::MouseMove, QPointF(100, bar.height() - 1),
                               Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(&bar, &mouseMoveNoBtn);

    // 鼠标移动且左键按下，靠近底部 => 触发窗口 resize 分支
    QMouseEvent mouseMoveLeft(QEvent::MouseMove, QPointF(100, bar.height() - 1),
                              Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(&bar, &mouseMoveLeft);

    // 鼠标移动到中部 => 走 ArrowCursor 分支
    QMouseEvent mouseMoveMid(QEvent::MouseMove, QPointF(100, 5),
                             Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(&bar, &mouseMoveMid);

    // 关闭垂直 resize，再移动一次 => 走 disabled 分支
    bar.setVerResized(false);
    QApplication::sendEvent(&bar, &mouseMoveMid);

    SUCCEED();
}
#endif
