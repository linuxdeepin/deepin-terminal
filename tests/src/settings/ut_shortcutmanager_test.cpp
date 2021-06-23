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

#include "ut_shortcutmanager_test.h"

#include "service.h"
#include "shortcutmanager.h"
#include "../stub.h"

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QDebug>
#include <QFile>
#include <QTimer>

#include <DSettings>

DCORE_USE_NAMESPACE

UT_ShortcutManager_Test::UT_ShortcutManager_Test()
{
    //Service中默认已经初始化了ShortcutManager
    if (!Service::instance()->property("isServiceInit").toBool()) {
        Service::instance()->setProperty("isServiceInit", true);
    }

    m_shortcutManager = ShortcutManager::instance();
}

#ifdef UT_SHORTCUTMANAGER_TEST
TEST_F(UT_ShortcutManager_Test, createBuiltinShortcutsFromConfig)
{
    m_shortcutManager->createBuiltinShortcutsFromConfig();
}


TEST_F(UT_ShortcutManager_Test, getCustomCommandActionList)
{
    QList<QAction *> &commandActionList = m_shortcutManager->getCustomCommandActionList();
    commandActionList.count();
}


TEST_F(UT_ShortcutManager_Test, addCustomCommand)
{
    QString key = QString(QChar('A'));
    QString shortcutKey = QString("Ctrl+Shift+%1").arg(key);
    QString cmdName = QString("cmd_001");
    QAction *newAction = new QAction;
    newAction->setText(cmdName);
    newAction->setShortcut(QKeySequence(shortcutKey));
    m_shortcutManager->addCustomCommand(*newAction);
}

TEST_F(UT_ShortcutManager_Test, checkActionIsExist)
{
    QString key = QString(QChar('A'));
    QString shortcutKey = QString("Ctrl+Shift+%1").arg(key);
    QString cmdName = QString("cmd_001");
    QAction *newAction = new QAction;
    newAction->setText(cmdName);
    newAction->setShortcut(QKeySequence(shortcutKey));
    m_shortcutManager->addCustomCommand(*newAction);
    m_shortcutManager->checkActionIsExist(*newAction);
}


TEST_F(UT_ShortcutManager_Test, checkActionIsExistForModify)
{
    QString key = QString(QChar('A'));
    QString shortcutKey = QString("Ctrl+Shift+%1").arg(key);
    QString cmdName = QString("cmd_001");
    QAction *newAction = new QAction;
    newAction->setText(cmdName);
    newAction->setShortcut(QKeySequence(shortcutKey));
    m_shortcutManager->checkActionIsExistForModify(*newAction);
}

TEST_F(UT_ShortcutManager_Test, findActionByKey)
{
    QString key = QString(QChar('A'));
    QString shortcutKey = QString("Ctrl+Shift+%1").arg(key);
    QString cmdName = QString("cmd_001");
    QAction *newAction = new QAction;
    newAction->setText(cmdName);
    newAction->setShortcut(QKeySequence(shortcutKey));
    m_shortcutManager->findActionByKey(cmdName);
}


TEST_F(UT_ShortcutManager_Test, isShortcutConflictInCustom)
{
    QString key = QString(QChar('A'));
    QString shortcutKey = QString("Ctrl+Shift+%1").arg(key);
    QString cmdName = QString("cmd_001");
    QAction *newAction = new QAction;
    newAction->setText(cmdName);
    newAction->setShortcut(QKeySequence(shortcutKey));

    m_shortcutManager->isShortcutConflictInCustom(cmdName, shortcutKey);
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
    m_shortcutManager->isValidShortcut(newCmdName, forCheckShortcutKey);

    s.reset(ADDR(ShortcutManager, checkShortcutValid));
    s.reset(ADDR(Utils, showShortcutConflictMsgbox));
}

TEST_F(UT_ShortcutManager_Test, checkShortcutValid)
{
    QString key = QString(QChar('A'));
    QString shortcutKey = QString("Ctrl+Shift+%1").arg(key);
    QString cmdName = QString("cmd_001");
    QAction *newAction = new QAction;
    newAction->setText(cmdName);
    newAction->setShortcut(QKeySequence(shortcutKey));

    QString reason;
    m_shortcutManager->checkShortcutValid(cmdName, shortcutKey, reason);
}

#endif
