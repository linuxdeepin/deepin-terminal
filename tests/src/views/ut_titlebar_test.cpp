// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
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
#endif
