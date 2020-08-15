#include "ut_mainwindow_test.h"

#include "service.h"
#include "mainwindow.h"

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>
#include <QApplication>
#include <QDesktopWidget>

UT_MainWindow_Test::UT_MainWindow_Test()
{
}

void UT_MainWindow_Test::SetUp()
{
    m_service = Service::instance();
    m_service->init();

    TermProperties normalTermProperty;
    normalTermProperty[QuakeMode] = false;
    normalTermProperty[SingleFlag] = true;
    m_normalWindow = new NormalWindow(normalTermProperty, nullptr);

    TermProperties quakeTermProperty;
    quakeTermProperty[QuakeMode] = true;
    m_quakeWindow = new QuakeWindow(quakeTermProperty, nullptr);
}

void UT_MainWindow_Test::TearDown()
{
    delete m_normalWindow;
    delete m_quakeWindow;
}

TEST_F(UT_MainWindow_Test, NormalWindowTest)
{
    EXPECT_EQ(MainWindow::m_MinWidth, 450);
    EXPECT_EQ(MainWindow::m_MinHeight, 250);

    EXPECT_NE(m_normalWindow, nullptr);

    m_normalWindow->show();

    EXPECT_GE(m_normalWindow->width(), MainWindow::m_MinWidth);
    EXPECT_GE(m_normalWindow->height(), MainWindow::m_MinHeight);
}

TEST_F(UT_MainWindow_Test, QuakeWindowTest)
{
    EXPECT_NE(m_quakeWindow, nullptr);

    m_quakeWindow->show();

    int desktopWidth = QApplication::desktop()->availableGeometry().width();
    EXPECT_EQ(m_quakeWindow->width(), desktopWidth);
    EXPECT_GE(m_quakeWindow->height(), MainWindow::m_MinHeight);
}

