
#ifndef UT_TERMWIDGETPAGE_TEST_H
#define UT_TERMWIDGETPAGE_TEST_H

#include "ut_defines.h"
#include "termproperties.h"

#include <gtest/gtest.h>

class Service;
class MainWindow;
class TermProperties;
class TermWidgetPage;
class UT_TermWidgetPage_Test : public ::testing::Test
{
public:
    UT_TermWidgetPage_Test();

public:
    //这里的几个函数都会自动调用

    //用于做一些初始化操作
    virtual void SetUp();

    //用于做一些清理操作
    virtual void TearDown();
public:
    Service *m_service = nullptr;

    //普通窗口
    MainWindow *m_normalWindow = nullptr;

    TermProperties m_normalTermProperty;
};

#endif // UT_TERMWIDGETPAGE_TEST_H

