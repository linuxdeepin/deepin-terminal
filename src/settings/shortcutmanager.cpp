// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shortcutmanager.h"
#include "termwidgetpage.h"
#include "mainwindow.h"
#include "settings.h"
#include "listview.h"
#include "settingio.h"
#include "qtcompat.h"

#include <QStandardPaths>
#include <QTextCodec>
#include <QSettings>
#include <QDebug>
#include <QDir>
#include <QLoggingCategory>

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(tsettings,"org.deepin.terminal.tsettings")
#else
Q_LOGGING_CATEGORY(tsettings,"org.deepin.terminal.tsettings",QtInfoMsg)
#endif

/* del by ut001121 zhangmeng 20201221 修复BUG58747
#define INI_FILE_CODEC QTextCodec::codecForName("UTF-8")*/

// this class only provided for convenience, do not do anything in the construct function,
// let the caller decided when to create the shortcuts.
ShortcutManager *ShortcutManager::m_instance = nullptr;

ShortcutManager::ShortcutManager(QObject *parent) : QObject(parent)
{
    // qCDebug(tsettings) << "Enter ShortcutManager::ShortcutManager";
    Utils::set_Object_Name(this);
}

ShortcutManager *ShortcutManager::instance()
{
    // qCDebug(tsettings) << "ShortcutManager instance requested";
    if (nullptr == m_instance) {
        qCDebug(tsettings) << "Creating new ShortcutManager instance";
        m_instance = new ShortcutManager();
    }
    return m_instance;
}

void ShortcutManager::initShortcuts()
{
    qCDebug(tsettings) << "Initializing built-in shortcuts";
    m_builtinShortcuts << "F1";
    m_builtinShortcuts << "Ctrl+C";
    m_builtinShortcuts << "Ctrl+D";

    /******** Modify by ut000439 wangpeili 2020-07-27: bug 39494   ****************/
    qCDebug(tsettings) << "Adding paste built-in shortcut";
    m_builtinShortcuts << QString(MainWindow::QKEYSEQUENCE_PASTE_BUILTIN);

    /******** Modify by ut001000 renfeixiang 2020-08-28:修改 bug 44477***************/
    qCDebug(tsettings) << "Adding copy built-in shortcut";
    m_builtinShortcuts << QString(MainWindow::QKEYSEQUENCE_COPY_BUILTIN);
    /********************* Modify by n014361 wangpeili End ************************/

    /******** Modify by ut000439 wangpeili 2020-07-20:  SP3 右键快捷键被内置   ****************/
    qCDebug(tsettings) << "Adding Alt+M shortcut";
    m_builtinShortcuts << "Alt+M";
    /********************* Modify by n014361 wangpeili End ************************/
    // 切换标签页，防止和ctrl+shift+1等冲突（无法识别）

    qCDebug(tsettings) << "Creating custom commands from config";
    createCustomCommandsFromConfig();
    qCDebug(tsettings) << "Setting up replace text mappings";
    m_mapReplaceText.insert("Ctrl+Shift+!", "Ctrl+Shift+1");
    m_mapReplaceText.insert("Ctrl+Shift+@", "Ctrl+Shift+2");
    m_mapReplaceText.insert("Ctrl+Shift+#", "Ctrl+Shift+3");
    m_mapReplaceText.insert("Ctrl+Shift+$", "Ctrl+Shift+4");
    m_mapReplaceText.insert("Ctrl+Shift+%", "Ctrl+Shift+5");
    m_mapReplaceText.insert("Ctrl+Shift+^", "Ctrl+Shift+6");
    m_mapReplaceText.insert("Ctrl+Shift+&", "Ctrl+Shift+7");
    m_mapReplaceText.insert("Ctrl+Shift+*", "Ctrl+Shift+8");
    m_mapReplaceText.insert("Ctrl+Shift+(", "Ctrl+Shift+9");
    m_mapReplaceText.insert("Return", "Enter");
}
ShortcutManager::~ShortcutManager()
{
    // qCDebug(tsettings) << "ShortcutManager destructor called";
    qDeleteAll(m_customCommandActionList.begin(), m_customCommandActionList.end());
    m_customCommandActionList.clear();
}


void ShortcutManager::initConnect(MainWindow *mainWindow)
{
    qCDebug(tsettings) << "Connecting shortcuts to MainWindow";
    for (auto &commandAction : m_customCommandActionList) {
        qCDebug(tsettings) << "Connecting action:" << commandAction->text();
        connect(commandAction, &QAction::triggered, mainWindow, &MainWindow::onCommandActionTriggered);
    }
    qCDebug(tsettings) << "Adding actions to main window";
    mainWindow->addActions(m_customCommandActionList);
}

void ShortcutManager::createCustomCommandsFromConfig()
{
    qCDebug(tsettings) << "Getting custom command base path";
    QDir customCommandBasePath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    if (!customCommandBasePath.exists()) {
        qCDebug(tsettings) << "custom command base path does not exist";
        return ;
    }

    QString customCommandConfigFilePath(customCommandBasePath.filePath("command-config.conf"));
    qCInfo(tsettings) << "load Custom Commands Config: " << customCommandConfigFilePath;
    if (!QFile::exists(customCommandConfigFilePath)) {
        qCDebug(tsettings) << "custom command config file does not exist";
        return ;
    }

    /***modify begin by ut001121 zhangmeng 20201221 修复BUG58747 远程管理和自定义命令名称中带/重启后会出现命令丢失***/
    /* add */
    qCDebug(tsettings) << "Creating USettings for custom commands";
    USettings commandsSettings(customCommandConfigFilePath);
    /***modify end by ut001121***/
    qCDebug(tsettings) << "Getting command groups";
    QStringList commandGroups = commandsSettings.childGroups();
    qCDebug(tsettings) << "Found" << commandGroups.size() << "command groups";
    for (const QString &commandName : commandGroups) {
        // qCDebug(tsettings) << "Processing command group:" << commandName;
        commandsSettings.beginGroup(commandName);
        if (!commandsSettings.contains("Command")) {
            // qCDebug(tsettings) << "command group missing Command key";
            continue;
        }

        // qCDebug(tsettings) << "Creating action for command:" << commandName;
        QAction *action = new QAction(commandName, this);
        action->setData(commandsSettings.value("Command").toString());  // make sure it is a QString
        if (commandsSettings.contains("Shortcut")) {
            // qCDebug(tsettings) << "processing shortcut for command";
            QVariant shortcutVariant = commandsSettings.value("Shortcut");
            if (QVariant::KeySequence == shortcutVariant.type()) {
                // qCDebug(tsettings) << "using KeySequence shortcut";
                action->setShortcut(shortcutVariant.convert(QMetaType::QKeySequence));
            } else if (QVariant::String == shortcutVariant.type()) {
                // qCDebug(tsettings) << "using String shortcut (compatibility)";
                //兼容deepin-terminal 旧的command-config.conf配置文件
                QString shortcutStr = shortcutVariant.toString().remove(QChar(' '));
                action->setShortcut(QKeySequence(shortcutStr));
            }
        }
        commandsSettings.endGroup();
        // qCDebug(tsettings) << "Adding action to custom command list";
        m_customCommandActionList.append(action);
    }

    if (SettingIO::rewrite) {
        qCDebug(tsettings) << "rewriting custom commands config";
        //删除自定义命令文件
        QFile fileTemp(customCommandConfigFilePath);
        if(!fileTemp.remove()) {
            qCWarning(tsettings) << " remove file error" << customCommandConfigFilePath << fileTemp.errorString();
        }

        //将内存数据写入配置文件
        qCDebug(tsettings) << "Writing memory data to config file";
        for (QAction *action : m_customCommandActionList) {
            saveCustomCommandToConfig(action, -1);
        }
    }
}

QList<QAction *> ShortcutManager::createBuiltinShortcutsFromConfig()
{
    // qCDebug(tsettings) << "Enter ShortcutManager::createBuiltinShortcutsFromConfig";
    QList<QAction *> actionList;
    return actionList;
}

QList<QAction *> &ShortcutManager::getCustomCommandActionList()
{
    // qCDebug(tsettings) << "Returning custom command action list with" << m_customCommandActionList.size() << "items";
    return m_customCommandActionList;
}

QAction *ShortcutManager::addCustomCommand(const QAction &action)
{
    qCDebug(tsettings) << "Adding custom command:" << action.text()
                      << "Shortcut:" << action.shortcut().toString();
    QAction *addAction = new QAction(action.text(), this);
    addAction->setData(action.data());
    addAction->setShortcut(action.shortcut());
    m_customCommandActionList.append(addAction);
    saveCustomCommandToConfig(addAction, -1);
    emit addCustomCommandSignal(addAction);

    return addAction;
}

QAction *ShortcutManager::checkActionIsExist(QAction &action)
{
    qCDebug(tsettings) << "Checking if action exists:" << action.text();
    QString strNewActionName = action.text();
    for (int i = 0; i < m_customCommandActionList.size(); i++) {
        QAction *currAction = m_customCommandActionList[i];
        if (currAction->text() == strNewActionName) {
            qCDebug(tsettings) << "Action found at index:" << i;
            return currAction;
        }
    }
    qCDebug(tsettings) << "Action not found";
    return nullptr;
}
/************************ Mod by m000743 sunchengxi 2020-04-21:自定义命令修改的异常问题 Begin************************/
QAction *ShortcutManager::checkActionIsExistForModify(QAction &action)
{
    qCDebug(tsettings) << "Checking if action exists for modify:" << action.text();
    QString strNewActionName = action.text();
    for (int i = 0; i < m_customCommandActionList.size(); i++) {
        QAction *currAction = m_customCommandActionList[i];
        if (currAction->text() == strNewActionName && action.data() == currAction->data() && action.shortcut() == currAction->shortcut()) {
            qCDebug(tsettings) << "Exact match found at index:" << i;
            return currAction;
        }
    }
    qCDebug(tsettings) << "No exact match found for modify";
    return nullptr;
}

QAction *ShortcutManager::findActionByKey(const QString &strKey)
{
    qCDebug(tsettings) << "Finding action by key:" << strKey;
    for (QAction *action : m_customCommandActionList) {
        if (action->text() == strKey) {
            qCInfo(tsettings) << "find action " << action;
            return action;
        }
    }

    qCInfo(tsettings) << "not find action name " << strKey;
    return nullptr;
}

void ShortcutManager::fillCommandListData(ListView *listview, const QString &strFilter)
{
    qCDebug(tsettings) << "Filling command list data with filter:" << strFilter;
    listview->clearData();
    QList<QAction *> &customCommandActionList = ShortcutManager::instance()->getCustomCommandActionList();
    //根据条件进行刷新，strFilter 为空时 全部刷新，根据名称或者命令模糊匹配到strFilter的条件进行刷新
    if (strFilter.isEmpty()) {
        qCDebug(tsettings) << "No filter, adding all commands";
        for (int i = 0; i < customCommandActionList.size(); i++) {
            QAction *curAction = customCommandActionList[i];
            QString strCmdName = curAction->text();
            QString strCmdShortcut = curAction->shortcut().toString();

            //listview->addItem(ItemFuncType_Item, strCmdName, strCmdShortcut);
            // 刷新列表,列表项中的值显示大写 down2up dzw 20201215
            listview->addItem(ItemFuncType_Item, strCmdName, Utils::converDownToUp(strCmdShortcut));
        }
    } else {
        qCDebug(tsettings) << "Filtering commands with filter:" << strFilter;
        for (int i = 0; i < customCommandActionList.size(); i++) {
            QAction *curAction = customCommandActionList[i];
            QString strCmdName = curAction->text();
            QString strCmdText = curAction->data().toString();
            QKeySequence keySeq = curAction->shortcut();
            QString strKeySeq = keySeq.toString();
            if (strCmdName.contains(strFilter, Qt::CaseSensitivity::CaseInsensitive)
                    || strCmdText.contains(strFilter, Qt::CaseSensitivity::CaseInsensitive)
                    || strKeySeq.contains(strFilter, Qt::CaseSensitivity::CaseInsensitive)) {
                qCDebug(tsettings) << "Command matches filter, adding:" << strCmdName;
                //listview->addItem(ItemFuncType_Item, strCmdName, strKeySeq);
                // 刷新列表,列表项中的值显示大写 down2up dzw 20201215
                listview->addItem(ItemFuncType_Item, strCmdName, Utils::converDownToUp(strKeySeq));
            }
        }
    }
}

/************************ Mod by m000743 sunchengxi 2020-04-21:自定义命令修改的异常问题 End  ************************/
bool ShortcutManager::isShortcutConflictInCustom(const QString &Name, const QString &Key)
{
    // qCDebug(tsettings) << "Checking shortcut conflict in custom commands for:" << Name << "key:" << Key;
    for (auto &currAction : m_customCommandActionList) {
        if (currAction->shortcut().toString() == Key) {
            if (Name != currAction->text()) {
                qCInfo(tsettings) << Name << Key << "is conflict with custom shortcut!";
                return true;
            }
        }
    }
    qCDebug(tsettings) << "No conflict found in custom commands";
    return false;
}

bool ShortcutManager::isValidShortcut(const QString &Name, const QString &Key)
{
    // qCDebug(tsettings) << "Validating shortcut:" << Name << "key:" << Key;
    QString reason;
    if (!checkShortcutValid(Name, Key, reason)) {
        // qCDebug(tsettings) << "shortcut is invalid";
        if (Key != "Esc") {
            qCDebug(tsettings) << "showing conflict message";
            Utils::showShortcutConflictMsgbox(reason);
        }
        return false;
    }
    // qCDebug(tsettings) << "Shortcut is valid";
    return true;
}

bool ShortcutManager::checkShortcutValid(const QString &Name, const QString &Key, QString &Reason)
{
    // qCDebug(tsettings) << "Checking shortcut validity for:" << Name << "key:" << Key;
    /******** Modify by ut000610 daizhengwen 2020-07-10:给'<'做兼容 html中'<'的转译为 &lt; Begin***************/
    QString key = Key;
    if (key.contains("<")) {
        qCDebug(tsettings) << "key contains '<', replacing";
        // 用<>断开 &lt; 与+号  +&lt; 会显示错误
        key.replace("<", "<>&lt;");
    }
    /********************* Modify by ut000610 daizhengwen End ************************/
    QString style = QString("<span style=\"color: rgba(255, 87, 54, 1);\">%1</span>").arg(key);

    // 单键
    if (0 == Key.count("+")) {
        qCDebug(tsettings) << "checking single key";
        //F1-F12是允许的，这个正则不够精确，但是没关系。
        REG_EXP regexp("^F[0-9]{1,2}$");
        if (!Key.contains(regexp)) {
            qCDebug(tsettings) << "single key is invalid";
            qCInfo(tsettings) << Key << "is invalid!";
            Reason = tr("The shortcut %1 is invalid, ")
                    .arg(style);
            return  false;
        }
    }
    // 小键盘单键都不允许
    REG_EXP regexpNum("^Num+.*");
    if (Key.contains(regexpNum)) {
        qCDebug(tsettings) << "numpad key is invalid";
        qCInfo(tsettings) << Key << "is invalid!";
        Reason = tr("The shortcut %1 is invalid, ")
                .arg(style);
        return  false;
    }
    // 内置快捷键都不允许
    if (m_builtinShortcuts.contains(Key)) {
        qCDebug(tsettings) << "key conflicts with builtin shortcut";
        qCInfo(tsettings) << Key << "is conflict with builtin shortcut!";
        Reason = tr("The shortcut %1 was already in use, ")
                .arg(style);
        return  false;
    }

    // 与设置里的快捷键冲突检测
    if (Settings::instance()->isShortcutConflict(Name, Key)) {
        qCDebug(tsettings) << "key conflicts with settings shortcut";
        Reason = tr("The shortcut %1 was already in use, ")
                .arg(style);
        return  false;
    }
    // 与自定义快捷键冲突检测
    if (isShortcutConflictInCustom(Name, Key)) {
        qCDebug(tsettings) << "key conflicts with custom shortcut";
        Reason = tr("The shortcut %1 was already in use, ")
                .arg(style);
        return  false;
    }
    qCDebug(tsettings) << "Shortcut is valid";
    return true;
}

void ShortcutManager::delCustomCommand(CustomCommandData itemData)
{
    // qCDebug(tsettings) << "Deleting custom command:" << itemData.m_cmdName;
    delCustomCommandToConfig(itemData);

    QString actionCmdName = itemData.m_cmdName;

    for (int i = 0; i < m_customCommandActionList.size(); i++) {
        QAction *currAction = m_customCommandActionList.at(i);
        QString currCmdName = currAction->text();
        if (actionCmdName == currCmdName) {
            qCDebug(tsettings) << "found command to delete";
            emit removeCustomCommandSignal(m_customCommandActionList.at(i));
            m_customCommandActionList.at(i)->deleteLater();
            m_customCommandActionList.removeAt(i);
            break;
        }
    }

}

void ShortcutManager::saveCustomCommandToConfig(QAction *action, int saveIndex)
{
    qCDebug(tsettings) << "Saving custom command to config:" << action->text()
                      << "Index:" << saveIndex;
    QDir customCommandBasePath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    if (!customCommandBasePath.exists()) {
        qCDebug(tsettings) << "creating config directory";
        customCommandBasePath.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    }

    QString customCommandConfigFilePath(customCommandBasePath.filePath("command-config.conf"));
    /***modify begin by ut001121 zhangmeng 20201221 修复BUG58747 远程管理和自定义命令名称中带/重启后会出现命令丢失***/
    /* add */
    USettings commandsSettings(customCommandConfigFilePath);
    /***modify end by ut001121***/
    commandsSettings.beginGroup(action->text());
    commandsSettings.setValue("Command", action->data());
    commandsSettings.setValue("Shortcut", action->shortcut().toString());
    commandsSettings.endGroup();

    if (saveIndex >= 0) {
        qCDebug(tsettings) << "updating action list at index" << saveIndex;
        QAction *saveAction = new QAction;
        saveAction->setText(action->text());
        saveAction->setData(action->data());
        saveAction->setShortcut(action->shortcut());
        m_customCommandActionList[saveIndex] = saveAction;
    }
}

int ShortcutManager::delCustomCommandToConfig(CustomCommandData itemData)
{
    qCDebug(tsettings) << "Deleting custom command:" << itemData.m_cmdName;
    QDir customCommandBasePath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    if (!customCommandBasePath.exists()) {
        qCDebug(tsettings) << "creating config directory";
        customCommandBasePath.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    }

    QString customCommandConfigFilePath(customCommandBasePath.filePath("command-config.conf"));
    /***modify begin by ut001121 zhangmeng 20201221 修复BUG58747 远程管理和自定义命令名称中带/重启后会出现命令丢失***/
    /* add */
    USettings commandsSettings(customCommandConfigFilePath);
    /***modify end by ut001121***/
    commandsSettings.remove(itemData.m_cmdName);

    int removeIndex = -1;
    for (int i = 0; i < m_customCommandActionList.size(); i++) {
        QAction *currAction = m_customCommandActionList[i];
        if (currAction->text() == itemData.m_cmdName) {
            qCDebug(tsettings) << "found command to remove at index" << i;
            removeIndex = i;
            break;
        }
    }
    return removeIndex;
}

