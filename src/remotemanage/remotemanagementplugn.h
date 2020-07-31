/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:     daizhengwen@uniontech.com
 *
 * Maintainer: daizhengwen@uniontech.com
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

/**
 * 远程管理插件管理类
 * 该类负责和主窗口交互，初始化远程管理的界面管理类
 * 初始化菜单按钮
 * 隐藏和显示窗口
 * 设置编码，退格键，删除键格式
 */
#ifndef REMOTEMANAGEMENTPLUGN_H
#define REMOTEMANAGEMENTPLUGN_H

#include "mainwindowplugininterface.h"
#include "remotemanagementtoppanel.h"

#include <QObject>
#include <QWidget>

class TermWidget;
class MainWindow;
class RemoteManagementPlugn : public MainWindowPluginInterface
{
    Q_OBJECT
public:
    explicit RemoteManagementPlugn(QObject *parent = nullptr);
    void initPlugin(MainWindow *mainWindow) override;
    QAction *titlebarMenu(MainWindow *mainWindow) override;

    RemoteManagementTopPanel *getRemoteManagementTopPanel();
    void initRemoteManagementTopPanel();
    QString createShellFile(ServerConfig *curServer);
    void hidePlugn();

    // 设置远程编码
    void setRemoteEncode(QString encode);
    // 设置退格模式
    void setBackspaceKey(TermWidget *term, QString backspaceKey);
    // 设置删除模式
    void setDeleteKey(TermWidget *term, QString deleteKey);
signals:
    void doHide();

public slots:
    void doCennectServer(ServerConfig *curServer);

private:
    MainWindow *m_mainWindow = nullptr;
    RemoteManagementTopPanel *m_remoteManagementTopPanel = nullptr;
};

#endif  // REMOTEMANAGEMENTPLUGN_H
