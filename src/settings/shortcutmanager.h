/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:     wangpeili <wangpeili@uniontech.com>
 *
 * Maintainer: wangpeili <wangpeili@uniontech.com>
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

#ifndef SHORTCUTMANAGER_H
#define SHORTCUTMANAGER_H

#include "define.h"
#include "utils.h"

#include <QList>
#include <QAction>

struct CustomCommandData {
    QString m_cmdName;
    QString m_cmdText;
    QString m_cmdShortcut;
};

class MainWindow;
class ListView;
/*******************************************************************************
 1. @类名:    ShortcutManager
 2. @作者:    ut000439 王培利
 3. @日期:    2020-07-31
 4. @说明:    全局快捷键管理

*******************************************************************************/
class ShortcutManager : public QObject
{
    Q_OBJECT
public:
    ShortcutManager(QObject *parent = nullptr);
    //void setMainWindow(MainWindow *curMainWindow);
    static ShortcutManager *instance();
    ~ShortcutManager();
    void initShortcuts();
    void initConnect(MainWindow *mainWindow);

    void createCustomCommandsFromConfig();
    QList<QAction *> createBuiltinShortcutsFromConfig();
    QList<QAction *> &getCustomCommandActionList();

    QAction *addCustomCommand(QAction &action);
    //void mainWindowAddAction(QAction *action);
    void delCustomCommand(CustomCommandData itemData);
    void saveCustomCommandToConfig(QAction *action, int saveIndex);
    int delCustomCommandToConfig(CustomCommandData itemData);
    // check same name of the action is exist
    QAction *checkActionIsExist(QAction &action);
    QAction *checkActionIsExistForModify(QAction &action);
    // 通过快捷键的唯一值找到action
    QAction *findActionByKey(const QString &strKey);
    // 填充列表项
    void fillCommandListData(ListView *listview, const QString &strFilter = "");

    // 判断快捷键是否合法可用，进行界面处理
    bool isValidShortcut(const QString &Name, const QString &Key);
    // 检测快捷键是否合法可用，无界面
    bool checkShortcutValid(const QString &Name, const QString &Key, QString &Reason);
    // 快捷键是否已被自定义设置
    bool isShortcutConflictInCustom(const QString &Name, const QString &Key);

    // 快捷键显示映射 如：Return 显示成 Enter
    QMap<QString, QString> m_mapReplaceText;

signals:
    void addCustomCommandSignal(QAction *newAction);
    void removeCustomCommandSignal(QAction *newAction);

private:
    QList<QAction *> m_customCommandActionList;
    QStringList m_builtinShortcuts;
    static ShortcutManager *m_instance;
};

#endif  // SHORTCUTMANAGER_H
