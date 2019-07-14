#include "shortcutmanager.h"

#include "termwidgetpage.h"
#include "mainwindow.h"
#include "settings.h"

#include <QDir>
#include <QSettings>
#include <QStandardPaths>

// this class only provided for convenience, do not do anything in the construct function,
// let the caller decided when to create the shortcuts.
ShortcutManager::ShortcutManager(MainWindow *parent) :
    QObject(parent),
    m_mainWindow(parent)
{
    // make sure it is NOT a nullptr since we'll use it all the time.
    Q_CHECK_PTR(parent);
}

void ShortcutManager::initShortcuts()
{
    m_customCommandList = createCustomCommandsFromConfig();
    m_builtinShortcuts = createBuiltinShortcutsFromConfig(); // use QAction or QShortcut ?

    m_mainWindow->addActions(m_customCommandList);
    m_mainWindow->addActions(m_builtinShortcuts);
}

QList<QAction *> ShortcutManager::createCustomCommandsFromConfig()
{
    QList<QAction *> actionList;

    QDir customCommandBasePath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    if (!customCommandBasePath.exists()) {
        return actionList;
    }

    QString customCommandConfigFilePath(customCommandBasePath.filePath("command-config.conf"));
    if (!QFile::exists(customCommandConfigFilePath)) {
        return actionList;
    }

    QSettings commandsSettings(customCommandConfigFilePath, QSettings::IniFormat);
    QStringList commandGroups = commandsSettings.childGroups();
    for (const QString &commandName : commandGroups) {
        commandsSettings.beginGroup(commandName);
        if (!commandsSettings.contains("Command")) continue;
        QAction * action = new QAction(commandName, this);
        action->setData(commandsSettings.value("Command").toString()); // make sure it is a QString
        if (commandsSettings.contains("Shortcut")) {
            QVariant shortcutVariant = commandsSettings.value("Shortcut");
            if (shortcutVariant.type() == QVariant::KeySequence) {
                action->setShortcut(shortcutVariant.convert(QMetaType::QKeySequence));
            } else if (shortcutVariant.type() == QVariant::String) {
                // to make it compatible to deepin-terminal config file.
                QString shortcutStr = shortcutVariant.toString().remove(QChar(' '));
                action->setShortcut(QKeySequence(shortcutStr));
            }
        }
        connect(action, &QAction::triggered, m_mainWindow, [this, action](){
            QString command = action->data().toString();
            if (!command.endsWith('\n')) {
                command.append('\n');
            }
            m_mainWindow->currentTab()->sendTextToCurrentTerm(command);
        });
        actionList.append(action);
    }

    return actionList;
}

QList<QAction *> ShortcutManager::createBuiltinShortcutsFromConfig()
{
    QList<QAction *> actionList;

    // TODO.

//    QAction *action = nullptr;

//    action = new QAction("__builtin_focus_nav_up", this);
//    // blumia: in Qt 5.7.1 (from Debian stretch) if we pass something like "Ctrl+Shift+Q" to QKeySequence then it won't works.
//    //         in Qt 5.11.3 (from Debian buster) it works fine.
//    action->setShortcut(QKeySequence("Alt+k"));

//    connect(action, &QAction::triggered, m_mainWindow, [this](){
//        TermWidgetPage *page = m_mainWindow->currentTab();
//        if (page) page->focusNavigation(Up);
//    });
//    actionList.append(action);

    return actionList;
}
