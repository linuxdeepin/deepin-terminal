// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
