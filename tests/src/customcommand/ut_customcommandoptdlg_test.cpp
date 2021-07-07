/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:     wangliang <wangliang@uniontech.com>
 *
 * Maintainer: wangliang <wangliang@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ut_customcommandoptdlg_test.h"

#include "ut_defines.h"
#include "customcommandoptdlg.h"

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QDebug>
#include <DApplicationHelper>

UT_CustomCommandOptDlg_Test::UT_CustomCommandOptDlg_Test()
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


#ifdef UT_CUSTOMCOMMANDOPTDLG_TEST

TEST_F(UT_CustomCommandOptDlg_Test, CustomCommandOptDlgTest)
{
    DApplicationHelper::instance()->setPaletteType(DApplicationHelper::DarkType);
    QSharedPointer<CustomCommandOptDlg> cmdDlg_new(new CustomCommandOptDlg, doDeleteLater);
    cmdDlg_new->setIconPixmap(QPixmap());
    cmdDlg_new->show();
    emit DApplicationHelper::instance()->themeTypeChanged(DApplicationHelper::DarkType);
    emit DApplicationHelper::instance()->themeTypeChanged(DApplicationHelper::LightType);
    DApplicationHelper::instance()->setPaletteType(DApplicationHelper::LightType);

    QSharedPointer<CustomCommandOptDlg> cmdDlg(new CustomCommandOptDlg, doDeleteLater);

    cmdDlg->show();

    QList<DLineEdit *> lineEditList = cmdDlg->findChildren<DLineEdit *>();
    EXPECT_EQ(lineEditList.size(), 2);

    DLineEdit *cmdNameEdit = lineEditList.first();
    DLineEdit *cmdTextEdit = lineEditList.last();

    QString str = Utils::getRandString();
    cmdNameEdit->setText(str.toLower());
    cmdTextEdit->setText("ls -al");

    DKeySequenceEdit *shortcutEdit = cmdDlg->findChild<DKeySequenceEdit *>();
    EXPECT_NE(shortcutEdit, nullptr);
    shortcutEdit->setKeySequence(QKeySequence("Ctrl+Shift+K"));
}

TEST_F(UT_CustomCommandOptDlg_Test, setCancelBtnText)
{
    QSharedPointer<CustomCommandOptDlg> cmdDlg(new CustomCommandOptDlg, doDeleteLater);

    cmdDlg->addCancelConfirmButtons();
    EXPECT_NE(cmdDlg->m_cancelBtn, nullptr);
    EXPECT_NE(cmdDlg->m_confirmBtn, nullptr);

    QString cancelText("Cancel");
    cmdDlg->setCancelBtnText(cancelText);
    EXPECT_EQ(cmdDlg->m_cancelBtn->text(), cancelText);
}

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

    QSharedPointer<CustomCommandOptDlg> cmdDlg(new CustomCommandOptDlg, doDeleteLater);
    cmdDlg->m_currItemData = new CustomCommandData;
    cmdDlg->m_currItemData->m_cmdName = "";
    cmdDlg->m_currItemData->m_cmdText = "";
    cmdDlg->m_currItemData->m_cmdShortcut = "";

    cmdDlg->slotAddSaveButtonClicked();

    cmdDlg->m_nameLineEdit->setText("cmd");
    cmdDlg->slotAddSaveButtonClicked();

    cmdDlg->m_commandLineEdit->setText("ls -al");
    cmdDlg->slotAddSaveButtonClicked();

    cmdDlg->m_shortCutLineEdit->setKeySequence(QKeySequence("Ctrl+Shift+T"));
    cmdDlg->slotAddSaveButtonClicked();

    cmdDlg->m_type = CustomCommandOptDlg::CCT_MODIFY;
    cmdDlg->slotAddSaveButtonClicked();

    {
        cmdDlg->m_bRefreshCheck = true;
        cmdDlg->slotAddSaveButtonClicked();

        cmdDlg->m_bRefreshCheck = false;
        cmdDlg->m_currItemData->m_cmdName = "cmd";
        cmdDlg->m_currItemData->m_cmdText = "ls -al";
        cmdDlg->m_currItemData->m_cmdShortcut = "Ctrl+Shift+T";
        cmdDlg->slotAddSaveButtonClicked();

        cmdDlg->m_nameLineEdit->setText("cmd_no");
        cmdDlg->slotAddSaveButtonClicked();

    }
}

TEST_F(UT_CustomCommandOptDlg_Test, slotRefreshData)
{
    QSharedPointer<CustomCommandOptDlg> cmdDlg(new CustomCommandOptDlg, doDeleteLater);

    cmdDlg->m_currItemData = new CustomCommandData;
    cmdDlg->m_currItemData->m_cmdName = "";
    cmdDlg->m_currItemData->m_cmdText = "";
    cmdDlg->m_currItemData->m_cmdShortcut = "";

    cmdDlg->m_nameLineEdit->setText("cmd");

    cmdDlg->m_commandLineEdit->setText("ls -al");

    QKeySequence keytmp = cmdDlg->m_shortCutLineEdit->setKeySequence(QKeySequence("Ctrl+Shift+T"));
    cmdDlg->slotAddSaveButtonClicked();

    cmdDlg->m_type = CustomCommandOptDlg::CCT_ADD;
    cmdDlg->slotRefreshData("cmd", "cmd2");

    cmdDlg->m_type = CustomCommandOptDlg::CCT_MODIFY;
    cmdDlg->slotRefreshData("cmd", "cmd2");
}



#endif
