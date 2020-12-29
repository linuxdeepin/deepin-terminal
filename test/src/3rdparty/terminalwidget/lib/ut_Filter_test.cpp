#include "ut_Filter_test.h"

#include "Filter.h"

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

/*******************************************************************************
 1. @函数:    Filter类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    clear单元测试
*******************************************************************************/
TEST_F(UT_Filter_Test, clear)
{
    TerminalImageFilterChain filterChain;
    filterChain.clear();
}

/*******************************************************************************
 1. @函数:    UrlFilter::HotSpot类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    setType单元测试
*******************************************************************************/
TEST_F(UT_Filter_Test, setType)
{
    UrlFilter::HotSpot *spot = new UrlFilter::HotSpot(0, 0, 10, 50);
    spot->setType(UrlFilter::HotSpot::Type::Marker);

    UrlFilter::HotSpot::Type type = spot->type();
    EXPECT_EQ(type, UrlFilter::HotSpot::Type::Marker);
}

/*******************************************************************************
 1. @函数:    UrlFilter::HotSpot类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    type单元测试
*******************************************************************************/
TEST_F(UT_Filter_Test, type)
{
    QStringList captureTextList;
    captureTextList << "http://mail.uniontech.com/";

    UrlFilter::HotSpot *spot = new UrlFilter::HotSpot(0, 0, 10, 50);
    spot->setCapturedTexts(captureTextList);

    UrlFilter::HotSpot::Type type = spot->type();
    EXPECT_EQ(type, UrlFilter::HotSpot::Type::Link);
}

/*******************************************************************************
 1. @函数:    UrlFilter::HotSpot类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    urlType单元测试
*******************************************************************************/
TEST_F(UT_Filter_Test, urlType)
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
}

/*******************************************************************************
 1. @函数:    TerminalImageFilterChain类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    addFilter单元测试
*******************************************************************************/
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

/*******************************************************************************
 1. @函数:    TerminalImageFilterChain类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    containsFilter单元测试
*******************************************************************************/
TEST_F(UT_Filter_Test, containsFilter)
{
    UrlFilter *urlFilter = new UrlFilter;

    TerminalImageFilterChain *filterChain = new TerminalImageFilterChain;
    filterChain->addFilter(urlFilter);

    bool isContain = filterChain->containsFilter(urlFilter);
    EXPECT_EQ(isContain, true);

    delete filterChain;
}

/*******************************************************************************
 1. @函数:    TerminalImageFilterChain类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    removeFilter单元测试
*******************************************************************************/
TEST_F(UT_Filter_Test, removeFilter)
{
    UrlFilter *urlFilter = new UrlFilter;

    TerminalImageFilterChain *filterChain = new TerminalImageFilterChain;
    filterChain->addFilter(urlFilter);

    filterChain->removeFilter(urlFilter);

    delete filterChain;
}

#endif
