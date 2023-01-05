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
