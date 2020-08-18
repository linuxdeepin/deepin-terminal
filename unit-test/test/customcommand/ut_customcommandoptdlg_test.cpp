#include "ut_customcommandoptdlg_test.h"

#include "../common/ut_defines.h"
#include "customcommandoptdlg.h"

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QDebug>

UT_CustomCommandOptDlg_Test::UT_CustomCommandOptDlg_Test()
{
}

void UT_CustomCommandOptDlg_Test::SetUp()
{
}

void UT_CustomCommandOptDlg_Test::TearDown()
{
}

#ifdef UT_CUSTOMCOMMANDOPTDLG_TEST
TEST_F(UT_CustomCommandOptDlg_Test, CustomCommandOptDlgTest)
{
    CustomCommandOptDlg cmdDlg;
    cmdDlg.show();

    QList<DLineEdit*> lineEditList = cmdDlg.findChildren<DLineEdit *>();
    EXPECT_EQ(lineEditList.size(), 2);

    DLineEdit *cmdNameEdit = lineEditList.first();
    DLineEdit *cmdTextEdit = lineEditList.last();

    QString str = Utils::getRandString();
    cmdNameEdit->setText(str.toLower());
    cmdTextEdit->setText("ls -al");

    DKeySequenceEdit *shortcutEdit = cmdDlg.findChild<DKeySequenceEdit *>();
    EXPECT_NE(shortcutEdit, nullptr);

    shortcutEdit->setKeySequence(QKeySequence("Ctrl+Shift+K"));

#ifdef ENABLE_UI_TEST
    QTest::qWait(1000);
#endif
}

#endif
