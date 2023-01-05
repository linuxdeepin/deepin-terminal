/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:  wangpeili<wangpeili@uniontech.com>
 *
 * Maintainer:wangpeili<wangpeili@uniontech.com>
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

#ifndef MAINWINDOWPLUGININTERFACE_H
#define MAINWINDOWPLUGININTERFACE_H

#include <QAction>

/*******************************************************************************
 1. @函数:    MainWindowPluginInterface
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:
*******************************************************************************/

class MainWindow;
class MainWindowPluginInterface : public QObject
{
    Q_OBJECT
public:
    explicit MainWindowPluginInterface(QObject *parent = nullptr) : QObject(parent)
    {
    }

    // 初始化插件
    virtual void initPlugin(MainWindow *mainWindow) = 0;
    // 设置标题上的菜单
    virtual QAction *titlebarMenu(MainWindow *mainWindow) = 0;

    // 获取标题名称
    QString getPluginName()
    {
        return m_pluginName;
    }

    // 插件名称
    QString m_pluginName;
    // 当前插件状态
    bool m_isShow = false;
};

#endif  // MAINWINDOWPLUGININTERFACE_H
