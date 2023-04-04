// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_pagesearchbar_test.h"

#include "pagesearchbar.h"
#include "termwidgetpage.h"
#include "service.h"
#include "utils.h"
#include "ut_stub_defines.h"

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QSignalSpy>
#include <QDebug>
#include <QMainWindow>

UT_PageSearchBar_Test::UT_PageSearchBar_Test()
{
}

void UT_PageSearchBar_Test::SetUp()
{
    if (!Service::instance()->property("isServiceInit").toBool()) {
        Service::instance()->setProperty("isServiceInit", true);
    }

    m_normalTermProperty[QuakeMode] = false;
    m_normalTermProperty[SingleFlag] = true;
    m_normalWindow = new NormalWindow(m_normalTermProperty, nullptr);
}

void UT_PageSearchBar_Test::TearDown()
{
    delete  m_normalWindow;
}

#ifdef UT_PAGESEARCHBAR_TEST
TEST_F(UT_PageSearchBar_Test, PageSearchBarTest)
{
    m_normalWindow->resize(800, 600);
    m_normalWindow->show();
    EXPECT_EQ(m_normalWindow->isVisible(), true);

    TermWidgetPage *termPage = m_normalWindow->currentPage();
    EXPECT_EQ(termPage->isVisible(), true);

    termPage->sendTextToCurrentTerm("ls -al\n");

    PageSearchBar *searchBar = termPage->m_findBar;
    termPage->showSearchBar(SearchBar_State::SearchBar_Show);
    EXPECT_EQ(searchBar->isVisible(), true);

    QString placeHolder = searchBar->m_originalPlaceHolder;
    searchBar->saveOldHoldContent();
    EXPECT_EQ(searchBar->m_searchEdit->placeHolder(), searchBar->m_originalPlaceHolder);
    searchBar->clearHoldContent();
    EXPECT_EQ(searchBar->m_searchEdit->placeHolder().length(), 0);
    searchBar->recoveryHoldContent();
    EXPECT_EQ(searchBar->m_searchEdit->placeHolder(), searchBar->m_originalPlaceHolder);

    QString searchTxt = searchBar->searchKeytxt();
    EXPECT_EQ(searchTxt.length(), 0);

    QString searchCmd = QString("rw");
    searchBar->m_searchEdit->setText(searchCmd);
    EXPECT_EQ(searchBar->m_searchEdit->text().length(), searchCmd.length());

    const int findNextCount = 2;
    //查找下一个'rw'字符串
    DIconButton *findNextButton = searchBar->m_findNextButton;
    QPoint clickPoint(3, 3);
    for (int i = 0; i < findNextCount; i++) {
        QSignalSpy spyNext(findNextButton, SIGNAL(clicked()));
        EXPECT_EQ(spyNext.count(), 0);
        QTest::mouseClick(findNextButton, Qt::LeftButton, Qt::NoModifier, clickPoint);
        EXPECT_EQ(spyNext.count(), 1);
    }

    const int findPrevCount = 2;
    //查找前一个rw
    for (int i = 0; i < findPrevCount; i++) {
        DIconButton *findPrevButton = searchBar->m_findPrevButton;
        QSignalSpy spyPrev(findPrevButton, SIGNAL(clicked()));
        EXPECT_EQ(spyPrev.count(), 0);
        QTest::mouseClick(findPrevButton, Qt::LeftButton, Qt::NoModifier, clickPoint);
        EXPECT_EQ(spyPrev.count(), 1);
    }
}

TEST_F(UT_PageSearchBar_Test, isFocus)
{
    TermWidgetPage *termPage = m_normalWindow->currentPage();
    ASSERT_TRUE(termPage);
    PageSearchBar *searchBar = termPage->m_findBar;
    ASSERT_TRUE(searchBar);

    UT_STUB_QWIDGET_HASFOCUS_CREATE;
    searchBar->isFocus();
    EXPECT_TRUE(UT_STUB_QWIDGET_HASFOCUS_RESULT);

    //keyPress
    UT_STUB_QWIDGET_HASFOCUS_PREPARE;
    QTest::keyPress(searchBar->window(), Qt::Key_Enter, Qt::NoModifier, 100);
    //lineEdit会判断hasFocus
    EXPECT_TRUE(UT_STUB_QWIDGET_HASFOCUS_RESULT);

    QTest::keyPress(searchBar->window(), Qt::Key_A);
}

TEST_F(UT_PageSearchBar_Test, keyPressEvent)
{
    TermWidgetPage *termPage = m_normalWindow->currentPage();
    ASSERT_TRUE(termPage);
    PageSearchBar *searchBar = termPage->m_findBar;
    ASSERT_TRUE(searchBar);

    UT_STUB_QWIDGET_HASFOCUS_CREATE;
    //查找前一个
    QTest::keyRelease(searchBar->window(), Qt::Key_Enter, Qt::NoModifier, 100);
    //查找下一个
    QTest::keyRelease(searchBar->window(), Qt::Key_Enter, Qt::ShiftModifier, 100);
    //default
    QTest::keyRelease(searchBar->window(), Qt::Key_A, Qt::NoModifier, 100);

    EXPECT_TRUE(UT_STUB_QWIDGET_HASFOCUS_RESULT);
}
#endif
