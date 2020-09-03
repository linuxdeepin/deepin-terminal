#include "ut_History_test.h"

#include "History.h"
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

#ifdef UT_History_TEST
TEST_F(UT_History_Test, HistoryNoneTest)
{
    HistoryType *history;

    history = new HistoryTypeNone();
    EXPECT_EQ(history->isEnabled(), false);
    EXPECT_EQ(history->isUnlimited(), true);
    EXPECT_EQ(history->maximumLineCount(), 0);
    delete history;
}

TEST_F(UT_History_Test, HistoryFileTest)
{
    HistoryType *history;

    history = new HistoryTypeFile();
    EXPECT_EQ(history->isEnabled(), true);
    EXPECT_EQ(history->isUnlimited(), true);
    EXPECT_EQ(history->maximumLineCount(), 0);
    delete history;
}

TEST_F(UT_History_Test, CompactHistoryTest)
{
    HistoryType *history;

    history = new CompactHistoryType(42);
    EXPECT_EQ(history->isEnabled(), true);
    EXPECT_EQ(history->isUnlimited(), false);
    EXPECT_EQ(history->maximumLineCount(), 42);
    delete history;
}

TEST_F(UT_History_Test, EmulationHistoryTest)
{
    Session* session = new Session();
    Emulation *emulation = session->emulation();

    const HistoryType &historyTypeDefault = emulation->history();
    EXPECT_EQ(historyTypeDefault.isEnabled(), false);
    EXPECT_EQ(historyTypeDefault.isUnlimited(), true);
    EXPECT_EQ(historyTypeDefault.maximumLineCount(), 0);

    emulation->setHistory(HistoryTypeNone());
    const HistoryType &historyTypeNone = emulation->history();
    EXPECT_EQ(historyTypeNone.isEnabled(), false);
    EXPECT_EQ(historyTypeNone.isUnlimited(), true);
    EXPECT_EQ(historyTypeNone.maximumLineCount(), 0);

    emulation->setHistory(HistoryTypeFile());
    const HistoryType &historyTypeFile = emulation->history();
    EXPECT_EQ(historyTypeFile.isEnabled(), true);
    EXPECT_EQ(historyTypeFile.isUnlimited(), true);
    EXPECT_EQ(historyTypeFile.maximumLineCount(), 0);

    emulation->setHistory(CompactHistoryType(42));
    const HistoryType &compactHistoryType = emulation->history();
    EXPECT_EQ(compactHistoryType.isEnabled(), true);
    EXPECT_EQ(compactHistoryType.isUnlimited(), false);
    EXPECT_EQ(compactHistoryType.maximumLineCount(), 42);

    delete session;
}

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
    EXPECT_EQ(historyTypeNone.isUnlimited(), true);
    EXPECT_EQ(historyTypeNone.maximumLineCount(), 0);

    delete historyScroll;

    // HistoryScrollFile
    QString logFileName = QString("~/ut_log_history");
    historyScroll = new HistoryScrollFile(logFileName);
    QVERIFY(historyScroll->hasScroll());
    EXPECT_EQ(historyScroll->getLines(), 0);
    EXPECT_EQ(historyScroll->getLineLen(0), 0);
    EXPECT_EQ(historyScroll->getLineLen(10), 0);

    const HistoryType &historyTypeFile = historyScroll->getType();
    EXPECT_EQ(historyTypeFile.isEnabled(), true);
    EXPECT_EQ(historyTypeFile.isUnlimited(), true);
    EXPECT_EQ(historyTypeFile.maximumLineCount(), 0);

    delete historyScroll;

    // CompactHistoryScroll
    historyScroll = new CompactHistoryScroll(42);
    QVERIFY(historyScroll->hasScroll());
    EXPECT_EQ(historyScroll->getLines(), 0);

    const HistoryType &compactHistoryType = historyScroll->getType();
    EXPECT_EQ(compactHistoryType.isEnabled(), true);
    EXPECT_EQ(compactHistoryType.isUnlimited(), false);
    EXPECT_EQ(compactHistoryType.maximumLineCount(), 42);

    delete historyScroll;
}

#endif
