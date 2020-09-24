#include "ut_myiconbutton_test.h"

#define private public
#include "myiconbutton.h"
#undef private

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QSignalSpy>
#include <QDebug>
#include <QMainWindow>
#include <DTitlebar>

DWIDGET_USE_NAMESPACE

UT_MyIconButton_Test::UT_MyIconButton_Test()
{
}

void UT_MyIconButton_Test::SetUp()
{
}

void UT_MyIconButton_Test::TearDown()
{
}

#ifdef UT_MYICONBUTTON_TEST

TEST_F(UT_MyIconButton_Test, TitleBarTest)
{
    MyIconButton *btn = new MyIconButton(nullptr);
    btn->setFixedSize(200, 20);
    EXPECT_NE(btn, nullptr);

    btn->show();
    EXPECT_EQ(btn->isVisible(), true);

    QPoint mousePoint(1, 1);
    QTest::mouseMove(btn, mousePoint, 200);

    QPoint windowPoint(1, 1);
    QEnterEvent enterEvent(mousePoint, windowPoint, windowPoint);
    QApplication::sendEvent(btn, &enterEvent);

    delete btn;
#ifdef ENABLE_UI_TEST
    QTest::qWait(200);
#endif
}

#endif
