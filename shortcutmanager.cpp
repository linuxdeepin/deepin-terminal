#include "shortcutmanager.h"
#include "termwidgetpage.h"
#include "mainwindow.h"
#include "settings.h"

#include <QDir>
#include "termqsettings.h"
#include <QStandardPaths>
#include <QDebug>

// this class only provided for convenience, do not do anything in the construct function,
// let the caller decided when to create the shortcuts.
ShortcutManager *ShortcutManager::m_instance = nullptr;

ShortcutManager::ShortcutManager(QObject *parent) : QObject(parent)
{
    // Q_UNUSED(parent);
    // make sure it is NOT a nullptr since we'll use it all the time.
    // Q_CHECK_PTR(parent);
}

ShortcutManager *ShortcutManager::instance()
{
    if (nullptr == m_instance) {
        m_instance = new ShortcutManager();
    }
    return m_instance;
}

void ShortcutManager::initShortcuts()
{
    m_customCommandActionList = createCustomCommandsFromConfig();
    m_builtinShortcuts = createBuiltinShortcutsFromConfig();  // use QAction or QShortcut ?

    m_mainWindow->addActions(m_customCommandActionList);
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

    TermQSettings commandsSettings(customCommandConfigFilePath, QSettings::IniFormat);
    commandsSettings.setDisableAutoSortSection(true);
    QStringList commandGroups = commandsSettings.childGroups();
    // qDebug() << commandGroups.size() << endl;
    for (const QString &commandName : commandGroups) {
        commandsSettings.beginGroup(commandName);
        if (!commandsSettings.contains("Command"))
            continue;
        QAction *action = new QAction(commandName, this);
        action->setData(commandsSettings.value("Command").toString());  // make sure it is a QString
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
        connect(action, &QAction::triggered, m_mainWindow, [this, action]() {
            QString command = action->data().toString();
            if (!command.endsWith('\n')) {
                command.append('\n');
            }
            m_mainWindow->currentTab()->sendTextToCurrentTerm(command);
        });
        commandsSettings.endGroup();
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
    //    // blumia: in Qt 5.7.1 (from Debian stretch) if we pass something like "Ctrl+Shift+Q" to QKeySequence then it
    //    won't works.
    //    //         in Qt 5.11.3 (from Debian buster) it works fine.
    //    action->setShortcut(QKeySequence("Alt+k"));

    //    connect(action, &QAction::triggered, m_mainWindow, [this](){
    //        TermWidgetPage *page = m_mainWindow->currentTab();
    //        if (page) page->focusNavigation(Up);
    //    });
    //    actionList.append(action);

    return actionList;
}

QList<QAction *> &ShortcutManager::getCustomCommandActionList()
{
    qDebug() << __FUNCTION__ << m_customCommandActionList.size();
    return m_customCommandActionList;
}

void ShortcutManager::setMainWindow(MainWindow *curMainWindow)
{
    m_mainWindow = curMainWindow;
}

QAction *ShortcutManager::addCustomCommand(QAction &action)
{
    QAction *addAction = new QAction(action.text(), this);
    addAction->setData(action.data());
    addAction->setShortcut(action.shortcut());
    m_customCommandActionList.append(addAction);
    connect(addAction, &QAction::triggered, m_mainWindow, [this, addAction]() {
        QString command = addAction->data().toString();
        if (!command.endsWith('\n')) {
            command.append('\n');
        }
        m_mainWindow->currentTab()->sendTextToCurrentTerm(command);
    });
    saveCustomCommandToConfig(addAction, -1);
    return addAction;
}

QAction *ShortcutManager::checkActionIsExist(QAction &action)
{
    QString strNewActionName = action.text();
    for (int i = 0; i < m_customCommandActionList.size(); i++) {
        QAction *currAction = m_customCommandActionList[i];
        if (strNewActionName == currAction->text()) {
            return currAction;
        }
    }
    return nullptr;
}

void ShortcutManager::delCustomCommand(QAction *action)
{
    delCustomCommandToConfig(action);

    QString actionCmdName = action->text();
    QString actionCmdText = action->data().toString();
    QString actionKeySeq = action->shortcut().toString();

    for (int i = 0; i < m_customCommandActionList.size(); i++) {
        QAction *currAction = m_customCommandActionList.at(i);
        QString currCmdName = currAction->text();
        QString currCmdText = currAction->data().toString();
        QString currKeySeq = currAction->shortcut().toString();
        if (actionCmdName == currCmdName
                && actionCmdText == currCmdText
                && actionKeySeq == currKeySeq) {
            m_customCommandActionList.removeAt(i);
            break;
        }
    }
}

void ShortcutManager::saveCustomCommandToConfig(QAction *action, int saveIndex)
{
    QDir customCommandBasePath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    if (!customCommandBasePath.exists()) {
        customCommandBasePath.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    }

    QString customCommandConfigFilePath(customCommandBasePath.filePath("command-config.conf"));
    TermQSettings commandsSettings(customCommandConfigFilePath, QSettings::IniFormat);
    commandsSettings.setDisableAutoSortSection(true);
    if (saveIndex >= 0) {
        commandsSettings.setOperationIndex(saveIndex);
    }
    commandsSettings.beginGroup(action->text());
    commandsSettings.setValue("Command", action->data());
    QString tmp = action->shortcut().toString();
    commandsSettings.setValue("Shortcut", action->shortcut().toString());
    commandsSettings.endGroup();

    qDebug() << "saveIndex:" << saveIndex;
    if (saveIndex >= 0) {
        QAction *saveAction = new QAction;
        saveAction->setText(action->text());
        saveAction->setData(action->data());
        saveAction->setShortcut(action->shortcut());
        m_customCommandActionList[saveIndex] = saveAction;
    }
}

int ShortcutManager::delCustomCommandToConfig(QAction *action)
{
    QDir customCommandBasePath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    if (!customCommandBasePath.exists()) {
        customCommandBasePath.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    }

    QString customCommandConfigFilePath(customCommandBasePath.filePath("command-config.conf"));
    TermQSettings commandsSettings(customCommandConfigFilePath, QSettings::IniFormat);
    int removeIndex = commandsSettings.remove(action->text());

    return removeIndex;
}
