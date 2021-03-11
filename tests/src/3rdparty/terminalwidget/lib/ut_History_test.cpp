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

#include "ut_History_test.h"

#include "history/HistoryType.h"
#include "history/HistoryTypeFile.h"
#include "history/HistoryTypeNone.h"
#include "history/HistoryScroll.h"
#include "history/HistoryScrollFile.h"
#include "history/HistoryScrollNone.h"
#include "history/compact/CompactHistoryType.h"
#include "history/compact/CompactHistoryScroll.h"
#include "Emulation.h"
#include "Session.h"

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QDebug>

using namespace Konsole;

UT_History_Test::UT_History_Test()
{
}

void UT_History_Test::SetUp()
{
}

void UT_History_Test::TearDown()
{
}

#ifdef UT_HISTORY_TEST

/*******************************************************************************
 1. @函数:    HistoryTypeNone类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    isEnabled/isUnlimited/maximumLineCount单元测试
*******************************************************************************/
TEST_F(UT_History_Test, HistoryTypeNone)
{
    HistoryType *history;

    history = new HistoryTypeNone();
    EXPECT_EQ(history->isEnabled(), false);
    EXPECT_EQ(history->maximumLineCount(), 0);
    delete history;
}

/*******************************************************************************
 1. @函数:    HistoryTypeFile类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    isEnabled/isUnlimited/maximumLineCount单元测试
*******************************************************************************/
TEST_F(UT_History_Test, HistoryTypeFile)
{
    HistoryType *history;

    history = new HistoryTypeFile();
    EXPECT_EQ(history->isEnabled(), true);
    EXPECT_EQ(history->isUnlimited(), true);
    delete history;
}

/*******************************************************************************
 1. @函数:    CompactHistoryType类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    isEnabled/isUnlimited/maximumLineCount单元测试
*******************************************************************************/
TEST_F(UT_History_Test, CompactHistoryTest)
{
    HistoryType *history;

    history = new CompactHistoryType(42);
    EXPECT_EQ(history->isEnabled(), true);
    EXPECT_EQ(history->isUnlimited(), false);
    EXPECT_EQ(history->maximumLineCount(), 42);
    delete history;
}

/*******************************************************************************
 1. @函数:    Emulation类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    Emulation history单元测试
*******************************************************************************/
TEST_F(UT_History_Test, history)
{
    Session *session = new Session();
    Emulation *emulation = session->emulation();

    const HistoryType &historyTypeDefault = emulation->history();
    EXPECT_EQ(historyTypeDefault.isEnabled(), false);
    EXPECT_EQ(historyTypeDefault.maximumLineCount(), 0);

    emulation->setHistory(HistoryTypeNone());
    const HistoryType &historyTypeNone = emulation->history();
    EXPECT_EQ(historyTypeNone.isEnabled(), false);
    EXPECT_EQ(historyTypeNone.maximumLineCount(), 0);

    emulation->setHistory(HistoryTypeFile());
    const HistoryType &historyTypeFile = emulation->history();
    EXPECT_EQ(historyTypeFile.isEnabled(), true);

    emulation->setHistory(CompactHistoryType(42));
    const HistoryType &compactHistoryType = emulation->history();
    EXPECT_EQ(compactHistoryType.isEnabled(), true);
    EXPECT_EQ(compactHistoryType.maximumLineCount(), 42);

    delete session;
}

/*******************************************************************************
 1. @函数:    HistoryScrollNone/HistoryScrollFile/CompactHistoryScroll
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    Emulation history单元测试
*******************************************************************************/
TEST_F(UT_History_Test, HistoryScrollTest)
{
    HistoryScroll *historyScroll;

    // HistoryScrollNone
    historyScroll = new HistoryScrollNone();
    QVERIFY(!historyScroll->hasScroll());
    EXPECT_EQ(historyScroll->getLines(), 0);
    EXPECT_EQ(historyScroll->getLineLen(0), 0);
    EXPECT_EQ(historyScroll->getLineLen(10), 0);

    const HistoryType &historyTypeNone = historyScroll->getType();
    EXPECT_EQ(historyTypeNone.isEnabled(), false);
    EXPECT_EQ(historyTypeNone.maximumLineCount(), 0);

    delete historyScroll;

    // HistoryScrollFile
    historyScroll = new HistoryScrollFile();
    QVERIFY(historyScroll->hasScroll());
    EXPECT_EQ(historyScroll->getLines(), 0);
    EXPECT_EQ(historyScroll->getLineLen(0), 0);
    EXPECT_EQ(historyScroll->getLineLen(10), 0);

    const HistoryType &historyTypeFile = historyScroll->getType();
    EXPECT_EQ(historyTypeFile.isEnabled(), true);

    delete historyScroll;

    // CompactHistoryScroll
    historyScroll = new CompactHistoryScroll(42);
    QVERIFY(historyScroll->hasScroll());
    EXPECT_EQ(historyScroll->getLines(), 0);

    const HistoryType &compactHistoryType = historyScroll->getType();
    EXPECT_EQ(compactHistoryType.isEnabled(), true);

    delete historyScroll;
}

#endif
