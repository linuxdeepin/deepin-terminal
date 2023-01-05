// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_shortcutmanager_test.h"
#include "service.h"
#include "shortcutmanager.h"
#include "../stub.h"

//dtk
#include <DSettings>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QDebug>
#include <QFile>
#include <QTimer>

DCORE_USE_NAMESPACE

UT_ShortcutManager_Test::UT_ShortcutManager_Test()
    : m_shortcutManager(nullptr)
    , newAction(nullptr)
{
}

void UT_ShortcutManager_Test::SetUp()
{

    //Service中默认已经初始化了ShortcutManager
    if (!Service::instance()->property("isServiceInit").toBool()) {
        Service::instance()->setProperty("isServiceInit", true);
    }

    m_shortcutManager = ShortcutManager::instance();

    QString key = QString(QChar('A'));
    QString shortcutKey = QString("Ctrl+Shift+%1").arg(key);
    QString cmdName = QString("cmd_001");
    newAction = new QAction;
    newAction->setText(cmdName);
    newAction->setShortcut(QKeySequence(shortcutKey));
}

void UT_ShortcutManager_Test::TearDown()
{
    delete newAction;
}

#ifdef UT_SHORTCUTMANAGER_TEST

TEST_F(UT_ShortcutManager_Test, getCustomCommandActionList)
{
    EXPECT_TRUE(m_shortcutManager->m_customCommandActionList ==
                m_shortcutManager->getCustomCommandActionList());
}


TEST_F(UT_ShortcutManager_Test, addCustomCommand)
{
    m_shortcutManager->addCustomCommand(*newAction);
    EXPECT_TRUE(m_shortcutManager->findActionByKey(newAction->text()));
}

TEST_F(UT_ShortcutManager_Test, checkActionIsExist)
{
    if(!m_shortcutManager->findActionByKey(newAction->text()))
        m_shortcutManager->addCustomCommand(*newAction);
    EXPECT_TRUE(m_shortcutManager->checkActionIsExist(*newAction));
}


TEST_F(UT_ShortcutManager_Test, checkActionIsExistForModify)
{
    if(!m_shortcutManager->findActionByKey(newAction->text()))
        m_shortcutManager->addCustomCommand(*newAction);
    EXPECT_TRUE(m_shortcutManager->checkActionIsExistForModify(*newAction));
}

TEST_F(UT_ShortcutManager_Test, findActionByKey)
{
    if(!m_shortcutManager->findActionByKey(newAction->text()))
        m_shortcutManager->addCustomCommand(*newAction);
    EXPECT_TRUE(m_shortcutManager->findActionByKey(newAction->text()));
}


TEST_F(UT_ShortcutManager_Test, isShortcutConflictInCustom)
{
    if(!m_shortcutManager->findActionByKey(newAction->text()))
        m_shortcutManager->addCustomCommand(*newAction);

    //名字不相同，快捷键相同，会返回true：快捷键冲突
    EXPECT_TRUE(m_shortcutManager->isShortcutConflictInCustom(newAction->text() + "1", newAction->shortcut().toString()));
}

bool stub_checkShortcutValid(const QString &Name, const QString &Key, QString &Reason)
{
    Q_UNUSED(Name)
    Q_UNUSED(Key)
    Q_UNUSED(Reason)
    return false;
}

bool stub_showShortcutConflictMsgbox(QString txt)
{
    Q_UNUSED(txt)
    return true;
}

TEST_F(UT_ShortcutManager_Test, isValidShortcut)
{
    Stub s;
    s.set(ADDR(ShortcutManager, checkShortcutValid), stub_checkShortcutValid);
    s.set(ADDR(Utils, showShortcutConflictMsgbox), stub_showShortcutConflictMsgbox);

    QString newCmdName = QString("cmd_new_%1").arg(Utils::getRandString());
    QString forCheckShortcutKey = QString("Ctrl+Alt+Shift+Tab+T");
    bool isValid = m_shortcutManager->isValidShortcut(newCmdName, forCheckShortcutKey);
    //重复快捷键，无效
    EXPECT_TRUE(isValid == false);
}

TEST_F(UT_ShortcutManager_Test, checkShortcutValid)
{
    if(!m_shortcutManager->findActionByKey(newAction->text()))
        m_shortcutManager->addCustomCommand(*newAction);
    //会冲突
    QString reason;
    bool isValid = m_shortcutManager->checkShortcutValid(newAction->text(), newAction->shortcut().toString(), reason);
    EXPECT_TRUE(isValid == false);
}

#endif
