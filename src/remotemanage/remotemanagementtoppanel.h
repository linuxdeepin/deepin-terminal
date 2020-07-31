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

class RemoteManagementTopPanel : public RightPanel
{
    Q_OBJECT
public:
    explicit RemoteManagementTopPanel(QWidget *parent = nullptr);
    void show();
    // 根据插件给的条件值设置焦点
    void setFocusInPanel();
protected:
    /******** Modify by nt001000 renfeixiang 2020-05-14:修改远程管理界面，在Alt+F2时，隐藏在显示，高度变大问题 Begin***************/
    //void resizeEvent(QResizeEvent *event) override;
    /******** Modify by nt001000 renfeixiang 2020-05-14:修改远程管理界面，在Alt+F2时，隐藏在显示，高度变大问题 End***************/
signals:
    void focusOut();
    void doConnectServer(ServerConfig *curServer);

public slots:
    void showSearchPanelFromRemotePanel(const QString &strFilter);
    void showServerConfigGroupPanelFromRemotePanel(const QString &strGroup, bool isFocusOn);

    void showRemotePanelFromGroupPanel(const QString &strGoupName, bool isFocusOn);
    void showSearchPanelFromGroupPanel(const QString &strGroup, const QString &strFilter);

    void showGroupPanelFromSearchPanel(const QString &strGroup, bool isKeyPress);
    void showRemoteManagementPanelFromSearchPanel();

    void slotShowGroupPanelFromSearchPanel(const QString &strGroup, bool isKeyPress);

private:
    void animationPrepare(CommonPanel *hidePanel, CommonPanel *showPanel);
    void panelLeftToRight(QPropertyAnimation *animation, QPropertyAnimation *animation1);
    void panelRightToLeft(QPropertyAnimation *animation, QPropertyAnimation *animation1);
    RemoteManagementPanel *m_remoteManagementPanel = nullptr;
    ServerConfigGroupPanel *m_serverConfigGroupPanel = nullptr;
    RemoteManagementSearchPanel *m_remoteManagementSearchPanel = nullptr;
    // 搜索条件
    QString m_filter;
    // 分组条件
    QString m_group;
};

#endif  // REMOTEMANAGEMENTTOPPANEL_H
