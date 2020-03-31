#include "shortcutmanager.h"
#include "termwidgetpage.h"
#include "mainwindow.h"
#include "settings.h"
#include "termqsettings.h"

#include <QStandardPaths>
#include <QTextCodec>
#include <QDebug>
#include <QDir>

#define INI_FILE_CODEC QTextCodec::codecForName("UTF-8")

// this class only provided for convenience, do not do anything in the construct function,
// let the caller decided when to create the shortcuts.
ShortcutManager *ShortcutManager::m_instance = nullptr;

ShortcutManager::ShortcutManager(QObject *parent) : QObject(parent), m_clipboardCommand("")
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
    // 快捷键初始化
    for (QString key : Settings::instance()->settings->keys())
    {
        qDebug() << key << Settings::instance()->settings->value(key);
        m_GloableShortctus[key] = Settings::instance()->settings->value(key).toString();
    }
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
    commandsSettings.setIniCodec(INI_FILE_CODEC);
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
    qDebug() << __FUNCTION__ << m_customCommandActionList;
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
    m_mainWindow->addAction(addAction);
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
/*******************************************************************************
 1. @函数:    shortcutConfirm
 2. @作者:    n014361 王培利
 3. @日期:    2020-03-26
 4. @说明:    全局检测是否冲突, 如果修改不成功，返回原来的快捷键
*******************************************************************************/
QString ShortcutManager::updateShortcut(const QString &Name, const QString &seq, bool loadMode)
{
    QString conflictName;
    qDebug()<<"updateShortcut " << Name<< seq <<loadMode;
    if(!isValidShortcut(seq))
    {
        return "";
    }
    for (QString item : m_GloableShortctus.keys())
    {
        if (m_GloableShortctus[item] == seq)
        {
            // 如果是自己键，值也一样，返回
            if (item == Name)
            {
                return Name;
            }
            // 如果冲突的值是初始值，不理会，直接设置
            if((m_GloableShortctus[item] == SHORTCUT_VALUE))
            {
                m_GloableShortctus[Name] = seq;
                return Name;
            }
            // 加载模式，不处理冲突，意味着配置文件出错了。
            if(loadMode)
            {
                qDebug() << "loadMode:"<<Name<<seq<<"conflict"<<item<< " set, not load config";
                return "";
            }
            // 如果某个已知项已经有了, 用户决定冲突是否覆盖。
            if (Utils::showShortcutConflictDialog(seq))
            {
                // 如果同意，则删除原，增加新，返回成功的
                m_GloableShortctus[item] = SHORTCUT_VALUE;
                m_GloableShortctus[Name] = seq;
                qDebug() << "m_GloableShortctus clear:" << item;
                qDebug() << "m_GloableShortctus set:" << Name << seq;
                return item;
            }
            else
            {
                // 如果不同意，返回原先的快捷键
                qDebug() << "change nothing" << Name;
                return "";
            }
        }
    }
    qDebug() << "first set m_GloableShortctus "<<Name << seq;
    m_GloableShortctus[Name] = seq;
    return Name;
}
/*******************************************************************************
 1. @函数:    getShortcutSet
 2. @作者:    n014361 王培利
 3. @日期:    2020-03-31
 4. @说明:    获取当前内存快捷键
*******************************************************************************/
QString ShortcutManager::getShortcutSet(const QString &Name)
{
    if (m_GloableShortctus.contains(Name))
    {
        return m_GloableShortctus[Name];
    }
    else
    {
        return "";
    }
}
/*******************************************************************************
 1. @函数:    判断快捷键是否合法
 2. @作者:    n014361 王培利
 3. @日期:    2020-03-31
 4. @说明:    目前单键除了F1-F12, 其它均不可以设置
*******************************************************************************/
bool ShortcutManager::isValidShortcut(const QString &Key)
{
    //F1-F12是允许的，这个正则不够精确，但是没关系。
    QRegExp regexp("^F[0-9]{1,2}$");
    if(Key.contains(regexp))
    {
        return true;
    }
    if(Key.count("+") == 0)
    {
        qDebug()<<Key<<"is invalid!";
        return  false;
    }
    return true;
}
void ShortcutManager::delCustomCommand(CustomCommandItemData itemData)
{
    delCustomCommandToConfig(itemData);

    QString actionCmdName = itemData.m_cmdName;
    QString actionCmdText = itemData.m_cmdText;
    QString actionKeySeq = itemData.m_cmdShortcut;

    for (int i = 0; i < m_customCommandActionList.size(); i++) {
        QAction *currAction = m_customCommandActionList.at(i);
        QString currCmdName = currAction->text();
        QString currCmdText = currAction->data().toString();
        QString currKeySeq = currAction->shortcut().toString();
        if (actionCmdName == currCmdName
                && actionCmdText == currCmdText
                && actionKeySeq == currKeySeq) {
            m_mainWindow->removeAction(m_customCommandActionList.at(i));
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
    commandsSettings.setIniCodec(INI_FILE_CODEC);
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
        qDebug()<<"old"<<m_customCommandActionList[saveIndex]->shortcut();
        m_mainWindow->removeAction(m_customCommandActionList[saveIndex]);
        m_customCommandActionList[saveIndex] = saveAction;
        m_mainWindow->addAction(saveAction);
        connect(saveAction, &QAction::triggered, m_mainWindow, [this, saveAction]() {
            QString command = saveAction->data().toString();
            if (!command.endsWith('\n')) {
                command.append('\n');
            }
            m_mainWindow->currentTab()->sendTextToCurrentTerm(command);
        });
        qDebug()<<"new"<<m_customCommandActionList[saveIndex]->shortcut();
    }
}

int ShortcutManager::delCustomCommandToConfig(CustomCommandItemData itemData)
{
    QDir customCommandBasePath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    if (!customCommandBasePath.exists()) {
        customCommandBasePath.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    }

    QString customCommandConfigFilePath(customCommandBasePath.filePath("command-config.conf"));
    TermQSettings commandsSettings(customCommandConfigFilePath, QSettings::IniFormat);
    commandsSettings.setIniCodec(INI_FILE_CODEC);
    int removeIndex = commandsSettings.remove(itemData.m_cmdName);

    return removeIndex;
}

void ShortcutManager::setClipboardCommandData(QString clipboardCommand)
{
    m_clipboardCommand = clipboardCommand;
}

QString ShortcutManager::getClipboardCommandData()
{
    return m_clipboardCommand;
}

