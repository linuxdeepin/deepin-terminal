// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_customcommandoptdlg_test.h"
#include "ut_defines.h"
#include "customcommandoptdlg.h"
#include "mainwindow.h"
#include "../stub.h"
#include "service.h"

//dtk
#include <DApplicationHelper>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QDebug>

//Google GTest 相关头文件
#include <gtest/gtest.h>

UT_CustomCommandOptDlg_Test::UT_CustomCommandOptDlg_Test()
{
    QAction *ac = new QAction;
    ac->setText(m_validCmdName);
    ac->setData(m_validCmdText);
    ac->setShortcut(m_validCmdShortcut);
    //ShortcutManager::instance()负责释放ac的空间
    ShortcutManager::instance()->m_customCommandActionList.append(ac);
}

UT_CustomCommandOptDlg_Test::~UT_CustomCommandOptDlg_Test()
{
}

void UT_CustomCommandOptDlg_Test::SetUp()
{
}

void UT_CustomCommandOptDlg_Test::TearDown()
{
}

static void doDeleteLater(CustomCommandOptDlg *obj)
{
    obj->deleteLater();
}

static bool ut_ShortcutManager_checkShortcutValid(const QString &, const QString &, QString &)
{
    return false;
}

#ifdef UT_CUSTOMCOMMANDOPTDLG_TEST

//TEST_F(UT_CustomCommandOptDlg_Test, CustomCommandOptDlgTest)
//{
//    DApplicationHelper::instance()->setPaletteType(DApplicationHelper::DarkType);
//    QSharedPointer<CustomCommandOptDlg> cmdDlg_new(new CustomCommandOptDlg, doDeleteLater);
//    cmdDlg_new->setIconPixmap(QPixmap());
//    cmdDlg_new->show();
//    emit DApplicationHelper::instance()->themeTypeChanged(DApplicationHelper::DarkType);
//    emit DApplicationHelper::instance()->themeTypeChanged(DApplicationHelper::LightType);
//    DApplicationHelper::instance()->setPaletteType(DApplicationHelper::LightType);

//    QSharedPointer<CustomCommandOptDlg> cmdDlg(new CustomCommandOptDlg, doDeleteLater);

//    cmdDlg->show();

//    QList<DLineEdit *> lineEditList = cmdDlg->findChildren<DLineEdit *>();
//    EXPECT_EQ(lineEditList.size(), 2);

//    DLineEdit *cmdNameEdit = lineEditList.first();
//    DLineEdit *cmdTextEdit = lineEditList.last();

//    QString str = Utils::getRandString();
//    cmdNameEdit->setText(str.toLower());
//    cmdTextEdit->setText("ls -al");

//    DKeySequenceEdit *shortcutEdit = cmdDlg->findChild<DKeySequenceEdit *>();
//    EXPECT_NE(shortcutEdit, nullptr);
//    shortcutEdit->setKeySequence(QKeySequence("Ctrl+Shift+K"));
//}

//TEST_F(UT_CustomCommandOptDlg_Test, setCancelBtnText)
//{
//    QSharedPointer<CustomCommandOptDlg> cmdDlg(new CustomCommandOptDlg, doDeleteLater);

//    cmdDlg->addCancelConfirmButtons();
//    EXPECT_NE(cmdDlg->m_cancelBtn, nullptr);
//    EXPECT_NE(cmdDlg->m_confirmBtn, nullptr);

//    QString cancelText("Cancel");
//    cmdDlg->setCancelBtnText(cancelText);
//    EXPECT_EQ(cmdDlg->m_cancelBtn->text(), cancelText);
//}

TEST_F(UT_CustomCommandOptDlg_Test, setConfirmBtnText)
{
    QSharedPointer<CustomCommandOptDlg> cmdDlg(new CustomCommandOptDlg, doDeleteLater);
    cmdDlg->addCancelConfirmButtons();
    EXPECT_NE(cmdDlg->m_cancelBtn, nullptr);
    EXPECT_NE(cmdDlg->m_confirmBtn, nullptr);

    QString confirmText("Add");
    cmdDlg->setConfirmBtnText(confirmText);
    EXPECT_EQ(cmdDlg->m_confirmBtn->text(), confirmText);
}

TEST_F(UT_CustomCommandOptDlg_Test, addCancelConfirmButtonsTest)
{
    QSharedPointer<CustomCommandOptDlg> cmdDlg(new CustomCommandOptDlg, doDeleteLater);
    cmdDlg->addCancelConfirmButtons();
    EXPECT_NE(cmdDlg->m_cancelBtn, nullptr);
    EXPECT_NE(cmdDlg->m_confirmBtn, nullptr);

    cmdDlg->m_shortCutLineEdit->setKeySequence(QKeySequence("Ctrl+T"));
    emit cmdDlg->m_cancelBtn->clicked(true);
    emit cmdDlg->m_confirmBtn->clicked(true);
    emit cmdDlg->m_shortCutLineEdit->editingFinished(QKeySequence("Ctrl+T"));
}

TEST_F(UT_CustomCommandOptDlg_Test, slotAddSaveButtonClicked)
{
    QKeySequence keySeq("Ctrl+T");
    QAction newAction(ShortcutManager::instance());
    newAction.setObjectName("CustomQAction");
    newAction.setText("cmd");
    newAction.setData("ls -al");
    newAction.setShortcut(keySeq);
    ShortcutManager::instance()->addCustomCommand(newAction);
    EXPECT_TRUE(ShortcutManager::instance()->findActionByKey(newAction.text()) != nullptr);

    QSharedPointer<CustomCommandOptDlg> cmdDlg(new CustomCommandOptDlg, doDeleteLater);
    cmdDlg->m_currItemData = new CustomCommandData;
    cmdDlg->m_currItemData->m_cmdName = "";
    cmdDlg->m_currItemData->m_cmdText = "";
    cmdDlg->m_currItemData->m_cmdShortcut = "";

    //add-条件：name="",cammand=""
    cmdDlg->slotAddSaveButtonClicked();
    EXPECT_TRUE(cmdDlg->m_nameLineEdit->text().isEmpty());

    //add-条件：name.length>MAX_NAME_LEN,cammand=""
    cmdDlg->m_nameLineEdit->setText(QString('A', MAX_NAME_LEN + 1));
    cmdDlg->slotAddSaveButtonClicked();
    EXPECT_TRUE(cmdDlg->m_nameLineEdit->text().count() > 0);

    //add-条件：name="cmd",cammand=""
    cmdDlg->m_nameLineEdit->setText("cmd");
    cmdDlg->slotAddSaveButtonClicked();
    EXPECT_TRUE(cmdDlg->m_nameLineEdit->text().count() > 0);

    //add-条件：name="cmd",cammand="ls -al"
    cmdDlg->m_commandLineEdit->setText("ls -al");
    const int cmdDlg_child_count = cmdDlg->children().count();
    cmdDlg->slotAddSaveButtonClicked();
    EXPECT_TRUE(1 == (cmdDlg->children().count() - cmdDlg_child_count));

    //modify-调试：m_bRefreshCheck=false,m_currItemData 为空,name<>name_old
    cmdDlg->m_type = CustomCommandOptDlg::CCT_MODIFY;
    cmdDlg->slotAddSaveButtonClicked();
    EXPECT_TRUE(2 == (cmdDlg->children().count() - cmdDlg_child_count));

    //modify-调试：m_bRefreshCheck=true,m_currItemData 为空,name<>name_old
    cmdDlg->m_bRefreshCheck = true;
    cmdDlg->slotAddSaveButtonClicked();
    EXPECT_TRUE(2 == (cmdDlg->children().count() - cmdDlg_child_count));

    //modify-调试：m_bRefreshCheck=false,m_currItemData 不为空,name<>name_old
    cmdDlg->m_bRefreshCheck = false;
    cmdDlg->m_currItemData->m_cmdName = this->m_validCmdName;
    cmdDlg->m_currItemData->m_cmdText = this->m_validCmdText;
    cmdDlg->m_currItemData->m_cmdShortcut = this->m_validCmdShortcut;
    cmdDlg->slotAddSaveButtonClicked();
    EXPECT_TRUE(2 == (cmdDlg->children().count() - cmdDlg_child_count));

    //modify-调试：m_bRefreshCheck=false,m_currItemData 不为空,name<>name_old
    cmdDlg->m_nameLineEdit->setText("cmd_no");
    cmdDlg->slotAddSaveButtonClicked();
    EXPECT_TRUE(2 == (cmdDlg->children().count() - cmdDlg_child_count));

    //modify-调试：m_bRefreshCheck=false,m_currItemData not empty,name=name_old
    cmdDlg->m_nameLineEdit->setText(m_validCmdName);
    cmdDlg->m_commandLineEdit->setText(m_validCmdText);
    cmdDlg->m_shortCutLineEdit->setKeySequence(QKeySequence(m_validCmdShortcut));
    cmdDlg->slotAddSaveButtonClicked();
    EXPECT_TRUE(2 == (cmdDlg->children().count() - cmdDlg_child_count));

}

TEST_F(UT_CustomCommandOptDlg_Test, slotRefreshData)
{
    QSharedPointer<CustomCommandOptDlg> cmdDlg(new CustomCommandOptDlg, doDeleteLater);

    cmdDlg->m_currItemData = new CustomCommandData;
    cmdDlg->m_currItemData->m_cmdName = "";
    cmdDlg->m_currItemData->m_cmdText = "";
    cmdDlg->m_currItemData->m_cmdShortcut = "";
    cmdDlg->m_nameLineEdit->setText(m_validCmdName);
    cmdDlg->m_commandLineEdit->setText(m_validCmdText);

    //新增一个窗口
    const int cmdDlg_child_count = cmdDlg->children().count();
    QKeySequence keytmp = cmdDlg->m_shortCutLineEdit->setKeySequence(QKeySequence(m_validCmdShortcut));
    cmdDlg->slotAddSaveButtonClicked();
    EXPECT_TRUE(1 == (cmdDlg->children().count() - cmdDlg_child_count));

    //新增的
    cmdDlg->m_type = CustomCommandOptDlg::CCT_ADD;
    cmdDlg->slotRefreshData(m_validCmdName, m_validCmdName + "1");
    EXPECT_TRUE(CustomCommandOptDlg::CCT_ADD == cmdDlg->m_type);

    //修改的， old != new
    cmdDlg->m_type = CustomCommandOptDlg::CCT_MODIFY;
    cmdDlg->slotRefreshData(m_validCmdName, m_validCmdName + "1");
    EXPECT_TRUE(CustomCommandOptDlg::CCT_MODIFY == cmdDlg->m_type);

    //修改的 old == new, 可以找到new action
    cmdDlg->m_currItemData->m_cmdName = m_validCmdName + "1";
    cmdDlg->m_currItemData->m_cmdText = "";
    cmdDlg->m_currItemData->m_cmdShortcut = "";
    cmdDlg->m_type = CustomCommandOptDlg::CCT_MODIFY;
    cmdDlg->slotRefreshData(m_validCmdName + "1", m_validCmdName);
    EXPECT_TRUE(CustomCommandOptDlg::CCT_MODIFY == cmdDlg->m_type);

    //修改的, old = new 没法找到new action
    cmdDlg->slotRefreshData(m_validCmdName, m_validCmdName + "1");
    EXPECT_TRUE(CustomCommandOptDlg::CCT_MODIFY == cmdDlg->m_type);
}

TEST_F(UT_CustomCommandOptDlg_Test, slotShortCutLineEditingFinished)
{
    Stub stub;
    stub.set(ADDR(ShortcutManager, checkShortcutValid), ut_ShortcutManager_checkShortcutValid);
    QSharedPointer<CustomCommandOptDlg> cmdDlg(new CustomCommandOptDlg, doDeleteLater);
    cmdDlg->slotShortCutLineEditingFinished(QKeySequence("Backspace"));
    //Backspace会设置lastCmdShortcut为""
    EXPECT_TRUE(cmdDlg->m_lastCmdShortcut.isEmpty());

    cmdDlg->slotShortCutLineEditingFinished(QKeySequence("Esc"));
    //还原最后lastCmdShortcut
    EXPECT_TRUE(cmdDlg->m_shortCutLineEdit->keySequence().toString() == cmdDlg->m_lastCmdShortcut);

    cmdDlg->m_type = CustomCommandOptDlg::CCT_MODIFY;
    cmdDlg->slotShortCutLineEditingFinished(QKeySequence());
    //还原最后lastCmdShortcut
    EXPECT_TRUE(cmdDlg->m_shortCutLineEdit->keySequence().toString() == cmdDlg->m_lastCmdShortcut);
}

TEST_F(UT_CustomCommandOptDlg_Test, initCommandFromClipBoardText)
{
    NormalWindow normal(TermProperties("/"), nullptr);
    CustomCommandOptDlg cmdDlg(CustomCommandOptDlg::CCT_ADD, nullptr, &normal);
    cmdDlg.initCommandFromClipBoardText();
    //会将selectedText赋值到m_commandLineEdit
    EXPECT_TRUE(cmdDlg.m_commandLineEdit->text() == normal.selectedText().trimmed());

}

TEST_F(UT_CustomCommandOptDlg_Test, closeRefreshDataConnection)
{
    QSharedPointer<CustomCommandOptDlg> cmdDlg(new CustomCommandOptDlg, doDeleteLater);
    int oldCount = Service::instance()->receivers(SIGNAL(refreshCommandPanel(QString, QString)));
    cmdDlg->closeRefreshDataConnection();
    int newCount = Service::instance()->receivers(SIGNAL(refreshCommandPanel(QString, QString)));
    //关联的槽函数-1
    EXPECT_TRUE(newCount == (oldCount - 1));
}

TEST_F(UT_CustomCommandOptDlg_Test, closeEvent)
{
    CustomCommandOptDlg cmdDlg;
    cmdDlg.close();
    //关闭时，会done(-1);
    EXPECT_TRUE(-1 == cmdDlg.result());
}

TEST_F(UT_CustomCommandOptDlg_Test, showShortcutConflict)
{
    CustomCommandOptDlg cmdDlg;
    cmdDlg.showShortcutConflictMsgbox("Return");
    //快捷键显示映射 如：Return 显示成 Enter
    EXPECT_TRUE(cmdDlg.m_shortcutConflictDialog->title().contains("Enter"));
}

TEST_F(UT_CustomCommandOptDlg_Test, checkSequence)
{
    Stub stub;
    stub.set(ADDR(ShortcutManager, checkShortcutValid), ut_ShortcutManager_checkShortcutValid);
    CustomCommandOptDlg cmdDlg;

    //空的sequence返回true
    EXPECT_TRUE(cmdDlg.checkSequence(QKeySequence()));
    //m_validCmdShortcut之前已被设置，故会冲突
    EXPECT_TRUE(!cmdDlg.checkSequence(QKeySequence(m_validCmdShortcut)));
}

TEST_F(UT_CustomCommandOptDlg_Test, slotCommandLineEditingFinished)
{
    CustomCommandOptDlg cmdDlg;
    cmdDlg.m_commandLineEdit->setText("");
    cmdDlg.slotCommandLineEditingFinished();

    //m_commandLineEdit为空时，placeholderText不为空
    EXPECT_TRUE(cmdDlg.m_commandLineEdit->lineEdit()->placeholderText().count() > 0);
}

#endif
