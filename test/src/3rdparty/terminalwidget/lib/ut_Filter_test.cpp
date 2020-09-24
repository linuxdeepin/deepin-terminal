#include "ut_Filter_test.h"

#define private public
#include "Filter.h"
#undef private

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QDebug>

using namespace Konsole;

UT_Filter_Test::UT_Filter_Test()
{
}

void UT_Filter_Test::SetUp()
{
}

void UT_Filter_Test::TearDown()
{
}

#ifdef UT_FILTER_TEST

TEST_F(UT_Filter_Test, clear)
{
    TerminalImageFilterChain filterChain;
    filterChain.clear();
}

TEST_F(UT_Filter_Test, RegExpFilterTest)
{
    Filter *regFilter = new RegExpFilter;
    EXPECT_EQ(regFilter != nullptr, true);

    delete regFilter;
}

TEST_F(UT_Filter_Test, UrlFilterTest)
{
    Filter *urlFilter = new UrlFilter;
    EXPECT_EQ(urlFilter != nullptr, true);
    delete urlFilter;
}

TEST_F(UT_Filter_Test, HotSpotTest)
{
    QStringList captureTextList;
    captureTextList << "https://www.baidu.com";
    UrlFilter::HotSpot *spot = new UrlFilter::HotSpot(0, 0, 10, 50);
    spot->setCapturedTexts(captureTextList);

    UrlFilter::HotSpot::UrlType urlType = spot->urlType();
    EXPECT_EQ(urlType, UrlFilter::HotSpot::UrlType::StandardUrl);

    captureTextList.clear();
    captureTextList << "name@email.com";
    spot->setCapturedTexts(captureTextList);
    urlType = spot->urlType();
    EXPECT_EQ(urlType, UrlFilter::HotSpot::UrlType::Email);

    captureTextList.clear();
    captureTextList << "aaa";
    spot->setCapturedTexts(captureTextList);
    urlType = spot->urlType();
    EXPECT_EQ(urlType, UrlFilter::HotSpot::UrlType::Unknown);

    UrlFilter::HotSpot::Type type = spot->type();
    EXPECT_EQ(type, UrlFilter::HotSpot::Type::Link);
}

TEST_F(UT_Filter_Test, addFilter)
{
    Filter *regFilter = new RegExpFilter;
    Filter *urlFilter = new UrlFilter;
    EXPECT_EQ(regFilter != nullptr, true);
    EXPECT_EQ(urlFilter != nullptr, true);

    TerminalImageFilterChain *filterChain = new TerminalImageFilterChain;
    EXPECT_EQ(filterChain != nullptr, true);
    filterChain->addFilter(regFilter);
    filterChain->addFilter(urlFilter);

    delete filterChain;
}

TEST_F(UT_Filter_Test, containsFilter)
{
    UrlFilter *urlFilter = new UrlFilter;

    TerminalImageFilterChain *filterChain = new TerminalImageFilterChain;
    filterChain->addFilter(urlFilter);

    bool isContain = filterChain->containsFilter(urlFilter);
    EXPECT_EQ(isContain, true);

    delete filterChain;
}

TEST_F(UT_Filter_Test, removeFilter)
{
    UrlFilter *urlFilter = new UrlFilter;

    TerminalImageFilterChain *filterChain = new TerminalImageFilterChain;
    filterChain->addFilter(urlFilter);

    filterChain->removeFilter(urlFilter);

    delete filterChain;
}

#endif
