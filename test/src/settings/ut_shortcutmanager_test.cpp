#include "ut_shortcutmanager_test.h"

#include "service.h"
#include "shortcutmanager.h"
#include "stub.h"

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
}

void UT_ShortcutManager_Test::SetUp()
{
    //Service中默认已经初始化了ShortcutManager
    if (!Service::instance()->property("isServiceInit").toBool()) {
        Service::instance()->init();
        Service::instance()->setProperty("isServiceInit", true);
    }

    m_shortcutManager = ShortcutManager::instance();
}

void UT_ShortcutManager_Test::TearDown()
{
}

#ifdef UT_SHORTCUTMANAGER_TEST
TEST_F(UT_ShortcutManager_Test, createBuiltinShortcutsFromConfig)
{
    QList<QAction *> builtinShortcuts = m_shortcutManager->createBuiltinShortcutsFromConfig();
//    EXPECT_GE(builtinShortcuts.count(), 0);
}


TEST_F(UT_ShortcutManager_Test, getCustomCommandActionList)
{
    QList<QAction *> &commandActionList = m_shortcutManager->getCustomCommandActionList();
    int commandCount = commandActionList.count();
//    EXPECT_GE(commandCount, 0);
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
//    EXPECT_GE(m_shortcutManager->getCustomCommandActionList().count(), commandCount + 1);
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
    QAction *currAction = m_shortcutManager->checkActionIsExist(*newAction);
//    EXPECT_NE(currAction, nullptr);
}


TEST_F(UT_ShortcutManager_Test, checkActionIsExistForModify)
{
    QString key = QString(QChar('A'));
    QString shortcutKey = QString("Ctrl+Shift+%1").arg(key);
    QString cmdName = QString("cmd_001");
    QAction *newAction = new QAction;
    newAction->setText(cmdName);
    newAction->setShortcut(QKeySequence(shortcutKey));
    QAction *currAction = m_shortcutManager->checkActionIsExistForModify(*newAction);
//    EXPECT_NE(currAction, nullptr);
}

TEST_F(UT_ShortcutManager_Test, findActionByKey)
{
    QString key = QString(QChar('A'));
    QString shortcutKey = QString("Ctrl+Shift+%1").arg(key);
    QString cmdName = QString("cmd_001");
    QAction *newAction = new QAction;
    newAction->setText(cmdName);
    newAction->setShortcut(QKeySequence(shortcutKey));
    QAction *currAction = m_shortcutManager->findActionByKey(cmdName);
//    EXPECT_NE(currAction, nullptr);
//    EXPECT_EQ(currAction->text(), cmdName);
}


TEST_F(UT_ShortcutManager_Test, isShortcutConflictInCustom)
{
    QString key = QString(QChar('A'));
    QString shortcutKey = QString("Ctrl+Shift+%1").arg(key);
    QString cmdName = QString("cmd_001");
    QAction *newAction = new QAction;
    newAction->setText(cmdName);
    newAction->setShortcut(QKeySequence(shortcutKey));

    bool isConflict = m_shortcutManager->isShortcutConflictInCustom(cmdName, shortcutKey);
//    EXPECT_EQ(isConflict, true);
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
//    EXPECT_EQ(isValid, true);

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
//    EXPECT_EQ(isValid, false);
}

#endif
