#include "ut_Screen_test.h"
#include "Screen.h"

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QDebug>

using namespace Konsole;

UT_Screen_Test::UT_Screen_Test()
{
}

void UT_Screen_Test::SetUp()
{
}

void UT_Screen_Test::TearDown()
{
}

void UT_Screen_Test::doScreenCopyVerify(QString &putToScreen, QString &expectedSelection)
{
    Screen screen(largeScreenLines, largeScreenColumns);

    for (const auto &lineCharacter : putToScreen) {
        screen.displayCharacter(lineCharacter.toLatin1());
    }

    screen.setSelectionStart(0, 0, false);
    screen.setSelectionEnd(largeScreenColumns, 0);
    EXPECT_EQ(screen.selectedText(Screen::PreserveLineBreaks), expectedSelection);
}

#ifdef UT_SCREEN_TEST

/*******************************************************************************
 1. @函数:    Screen类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    setSelectionStart/setSelectionEnd单元测试
*******************************************************************************/
TEST_F(UT_Screen_Test, selection)
{
    QString putToScreen = QStringLiteral("0123456789abcde");
    QString expectedSelection = QStringLiteral("0123456789abcde\n");
    doScreenCopyVerify(putToScreen, expectedSelection);

    putToScreen = "";
    //超过屏幕一行的字符个数 (屏幕一行最多1200字符，这里使用1300字符)
    for (int i = 0; i < 120; ++i) {
        putToScreen.append(QStringLiteral("0123456789"));
    }
    expectedSelection = putToScreen.left(1000);
    doScreenCopyVerify(putToScreen, expectedSelection);
}

#endif
