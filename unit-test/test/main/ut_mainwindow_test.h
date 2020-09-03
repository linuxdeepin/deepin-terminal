
#ifndef UT_MAINWINDOW_TEST_H
#define UT_MAINWINDOW_TEST_H

#include "ut_defines.h"
#include "termproperties.h"

#include <gtest/gtest.h>

class Service;
class MainWindow;
class UT_MainWindow_Test : public ::testing::Test
{
public:
    UT_MainWindow_Test();

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

    //雷神窗口
    MainWindow *m_quakeWindow = nullptr;

    TermProperties m_normalTermProperty;
    TermProperties m_quakeTermProperty;
};

#endif // UT_MAINWINDOW_TEST_H

