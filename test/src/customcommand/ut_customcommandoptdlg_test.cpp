#include "ut_customcommandoptdlg_test.h"

#include "ut_defines.h"
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

    QList<DLineEdit *> lineEditList = cmdDlg.findChildren<DLineEdit *>();
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
    QTest::qWait(UT_WAIT_TIME);
#endif
}

TEST_F(UT_CustomCommandOptDlg_Test, setCancelBtnText)
{
    CustomCommandOptDlg cmdDlg;
    cmdDlg.addCancelConfirmButtons();
    EXPECT_NE(cmdDlg.m_cancelBtn, nullptr);
    EXPECT_NE(cmdDlg.m_confirmBtn, nullptr);

    QString cancelText("Cancel");
    cmdDlg.setCancelBtnText(cancelText);
    EXPECT_EQ(cmdDlg.m_cancelBtn->text(), cancelText);
}

TEST_F(UT_CustomCommandOptDlg_Test, setConfirmBtnText)
{
    CustomCommandOptDlg cmdDlg;
    cmdDlg.addCancelConfirmButtons();
    EXPECT_NE(cmdDlg.m_cancelBtn, nullptr);
    EXPECT_NE(cmdDlg.m_confirmBtn, nullptr);

    QString confirmText("Add");
    cmdDlg.setConfirmBtnText(confirmText);
    EXPECT_EQ(cmdDlg.m_confirmBtn->text(), confirmText);
}

TEST_F(UT_CustomCommandOptDlg_Test, slotAddSaveButtonClicked)
{
    CustomCommandOptDlg cmdDlg;
    cmdDlg.m_currItemData = new CustomCommandData;
    cmdDlg.m_currItemData->m_cmdName = "";
    cmdDlg.m_currItemData->m_cmdText = "";
    cmdDlg.m_currItemData->m_cmdShortcut = "";

    cmdDlg.slotAddSaveButtonClicked();

    cmdDlg.m_nameLineEdit->setText("cmd");
    cmdDlg.slotAddSaveButtonClicked();

    cmdDlg.m_commandLineEdit->setText("ls -al");
    cmdDlg.slotAddSaveButtonClicked();

    QKeySequence keytmp = cmdDlg.m_shortCutLineEdit->setKeySequence(QKeySequence("Ctrl+Shift+T"));
    cmdDlg.slotAddSaveButtonClicked();

    cmdDlg.m_type = CustomCommandOptDlg::CCT_MODIFY;
    cmdDlg.slotAddSaveButtonClicked();
}

TEST_F(UT_CustomCommandOptDlg_Test, slotRefreshData)
{
    CustomCommandOptDlg cmdDlg;
    cmdDlg.m_currItemData = new CustomCommandData;
    cmdDlg.m_currItemData->m_cmdName = "";
    cmdDlg.m_currItemData->m_cmdText = "";
    cmdDlg.m_currItemData->m_cmdShortcut = "";

    cmdDlg.m_nameLineEdit->setText("cmd");

    cmdDlg.m_commandLineEdit->setText("ls -al");

    QKeySequence keytmp = cmdDlg.m_shortCutLineEdit->setKeySequence(QKeySequence("Ctrl+Shift+T"));
    cmdDlg.slotAddSaveButtonClicked();

    cmdDlg.m_type = CustomCommandOptDlg::CCT_ADD;
    cmdDlg.slotRefreshData("cmd", "cmd2");

    cmdDlg.m_type = CustomCommandOptDlg::CCT_MODIFY;
    cmdDlg.slotRefreshData("cmd", "cmd2");
}



#endif
