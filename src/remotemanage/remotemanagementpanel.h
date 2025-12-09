// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/*******************************************************************************
 1. @类名:    RemoteManagementPanel
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    远程管理初始界面，依次显示搜索框，远程列表，添加服务器按钮
             搜索框只有远程列表里的项超过两个才显示
             远程列表里的项分两类：
             1）分组项：该项内有同一分组的所有远程项
             2）远程项：该项内可以点击连接远程服务器
*******************************************************************************/
#ifndef REMOTEMANAGEMENTPANEL_H
#define REMOTEMANAGEMENTPANEL_H

#include "commonpanel.h"
#include "serverconfigoptdlg.h"
#include "groupconfigoptdlg.h"
#include "listview.h"

#include <QWidget>

class RemoteManagementPanel : public CommonPanel
{
    Q_OBJECT
public:
    explicit RemoteManagementPanel(QWidget *parent = nullptr);
    /**
     * @brief 刷新远程管理面板
     * @author ut000610 daizhengwen
     */
    void refreshPanel();
    /**
     * @brief 将焦点设置进入panel，有搜索框焦点进搜索框，没搜索框，焦点进入列表，没列表，焦点进添加按钮
     * @author ut000610 戴正文
     */
    void setFocusInPanel();
    /**
     * @brief 设置焦点从分组界面返回时的位置
     * @author ut000610 戴正文
     * @param strGroup 组名
     */
    void setFocusBack(const QString &strGroup);
    /**
     * @brief 清空列表的选中状态
     * @author ut000610 戴正文
     */
    void clearListFocus();
    /**
     * @brief 获取列表中当前焦点的位置
     * @author ut000610 戴正文
     * @return
     */
    int getListIndex();

signals:
    // 显示搜索界面
    void showSearchPanel(const QString &strFilter);
    // 显示分组,根据布尔值判断是否有焦点
    void showGroupPanel(const QString &strGroup, bool isFocusOn);
    // 连接远程
    void doConnectServer(ServerConfig *curItemServer);

public slots:
    /**
     * @brief 显示远程管理当前搜索结果
     * @author ut000610 daizhengwen
     */
    void showCurSearchResult();
    /**
     * @brief 显示远程管理添加服务器配置界面
     * @author ut000610 daizhengwen
     */
    void showAddServerConfigDlg();
    /**
     * @brief 显示远程管理添加分组配置界面
     * @author Archie Meng
     */
    void showAddGroupConfigDlg(const QString &groupName = QStringLiteral(""));
    /**
     * @brief 刷新远程管理搜索状态
     * @author ut000610 daizhengwen
     */
    void refreshSearchState();
    /**
     * @brief 远程项被点击，连接远程管理
     * @author ut000610 戴正文
     * @param key 远程项
     */
    void onItemClicked(const QString &key);

private:
    /**
     * @brief 初始化远程管理UI界面
     * @author ut000610 daizhengwen
     */
    void initUI();

private:
    ListView *m_listWidget = nullptr;
    DLabel *m_textLabel = nullptr;
    DLabel *m_imageLabel = nullptr;
    QVBoxLayout *m_backLayout = nullptr;
};

#endif  // REMOTEMANAGEMENTPANEL_H
