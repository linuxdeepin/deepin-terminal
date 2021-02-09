
#ifndef UT_TERMWIDGETPAGE_TEST_H
#define UT_TERMWIDGETPAGE_TEST_H

#include "ut_defines.h"
#include "termproperties.h"

#include <gtest/gtest.h>

class Service;
class MainWindow;
class TermProperties;

class UT_TermWidgetPage_Test : public ::testing::Test
{
public:
    UT_TermWidgetPage_Test();

    ~UT_TermWidgetPage_Test();

private:
    Service *m_service = nullptr;

    //普通窗口
    MainWindow *m_normalWindow = nullptr;

    TermProperties m_normalTermProperty;
};

#endif // UT_TERMWIDGETPAGE_TEST_H

