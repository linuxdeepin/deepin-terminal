#include "ut_shortcutmanager_test.h"

#include "service.h"
#include "shortcutmanager.h"

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
TEST_F(UT_ShortcutManager_Test, SettingsTest)
{
    QList<QAction *> builtinShortcuts = m_shortcutManager->createBuiltinShortcutsFromConfig();
    EXPECT_GE(builtinShortcuts.count(), 0);

    QList<QAction *> &commandActionList = m_shortcutManager->getCustomCommandActionList();
    int commandCount = commandActionList.count();
    EXPECT_GE(commandCount, 0);

    QString key = QString(QChar('A'));
    QString shortcutKey = QString("Ctrl+Shift+%1").arg(key);
    QString cmdName = QString("cmd_001");
    QAction *newAction = new QAction;
    newAction->setText(cmdName);
    newAction->setShortcut(QKeySequence(shortcutKey));
    m_shortcutManager->addCustomCommand(*newAction);

    EXPECT_GE(m_shortcutManager->getCustomCommandActionList().count(), commandCount + 1);

    QAction *currAction = m_shortcutManager->checkActionIsExist(*newAction);
    EXPECT_NE(currAction, nullptr);

    currAction = m_shortcutManager->checkActionIsExistForModify(*newAction);
    EXPECT_NE(currAction, nullptr);

    currAction = m_shortcutManager->findActionByKey(cmdName);
    EXPECT_NE(currAction, nullptr);
    EXPECT_EQ(currAction->text(), cmdName);

#ifdef ENABLE_UI_TEST
    bool isConflict = m_shortcutManager->isShortcutConflictInCustom(cmdName, shortcutKey);
    EXPECT_EQ(isConflict, true);

    QString newCmdName = QString("cmd_new_%1").arg(Utils::getRandString());
    QString forCheckShortcutKey = QString("Ctrl+Alt+Shift+Tab+T");
    bool isValid = m_shortcutManager->isValidShortcut(newCmdName, forCheckShortcutKey);
    EXPECT_EQ(isValid, true);

    QString reason;
    isValid = m_shortcutManager->checkShortcutValid(cmdName, shortcutKey, reason);
    EXPECT_EQ(isValid, false);

    //要自己退出，否则对话框窗口会一直阻塞
    QTimer::singleShot(1000, qApp, [ = ] {
        qApp->exit();
    });

    isValid = m_shortcutManager->isValidShortcut(cmdName, shortcutKey);
    EXPECT_EQ(isValid, false);
#endif
}
#endif
