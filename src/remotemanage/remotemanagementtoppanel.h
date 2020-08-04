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
 * 远程管理界面管理类
 * 主要负责界面切换和动画效果
 */
#ifndef REMOTEMANAGEMENTTOPPANEL_H
#define REMOTEMANAGEMENTTOPPANEL_H

#include "rightpanel.h"
#include "commonpanel.h"
#include "remotemanagementpanel.h"
#include "serverconfiggrouppanel.h"
#include "remotemanagementsearchpanel.h"
#include "serverconfigmanager.h"

#include <QPropertyAnimation>
#include <QWidget>
#include <QStack>

class RemoteManagementTopPanel : public RightPanel
{
    Q_OBJECT
public:
    explicit RemoteManagementTopPanel(QWidget *parent = nullptr);
    void show();
    // 根据插件给的条件值设置焦点
    void setFocusInPanel();

public slots:
    // 显示搜索框
    void showSearchPanel(const QString &strFilter);
    // 显示分组界面
    void showGroupPanel(const QString &strGroupName, bool isFocusOn);
    // 显示前一个界面（返回）
    void showPrevPanel();

signals:
    void focusOut();
    void doConnectServer(ServerConfig *curServer);

private:
    // 远程主界面
    RemoteManagementPanel *m_remoteManagementPanel = nullptr;
    // 远程分组界面
    ServerConfigGroupPanel *m_serverConfigGroupPanel = nullptr;
    // 远程搜索界面
    RemoteManagementSearchPanel *m_remoteManagementSearchPanel = nullptr;
    // 搜索条件
    QString m_filter;
    // 分组条件
    QString m_group;
    // 当前界面
    ServerConfigManager::PanelType m_currentPanelType;
    // 记录前一个界面
    QStack<ServerConfigManager::PanelType> m_prevPanelStack;

    // 设置平面显示状态
    void setPanelShowState(ServerConfigManager::PanelType panelType);
    // 平面从左向右显示
    void panelLeftToRight(QPropertyAnimation *animation, QPropertyAnimation *animation1);
    // 平面从右向左显示
    void panelRightToLeft(QPropertyAnimation *animation, QPropertyAnimation *animation1);
};

#endif  // REMOTEMANAGEMENTTOPPANEL_H
