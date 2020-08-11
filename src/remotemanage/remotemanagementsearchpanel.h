/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:  daizhengwen<daizhengwen@uniontech.com>
 *
 * Maintainer: daizhengwen<daizhengwen@uniontech.com>
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
 * 远程管理搜索界面
 * 主要由返回按钮， 搜索条件显示框和搜索结果列表构成
 */
#ifndef REMOTEMANAGEMENTSEARCHPANEL_H
#define REMOTEMANAGEMENTSEARCHPANEL_H

#include "commonpanel.h"
#include "serverconfigmanager.h"

#include <QWidget>

class ListView;
class RemoteManagementSearchPanel : public CommonPanel
{
    Q_OBJECT
public:

    explicit RemoteManagementSearchPanel(QWidget *parent = nullptr);
    void refreshDataByGroupAndFilter(const QString &strGroup, const QString &strFilter);
    void refreshDataByFilter(const QString &strFilter);
    // 清除界面所有焦点
    void clearAllFocus();
    // 设置焦点返回
    void setFocusBack(const QString &strGroup, bool isFocusOn, int prevIndex);
    // 获取列表的index
    int getListIndex();

signals:
    // 连接远程
    void doConnectServer(ServerConfig *curItemServer);
    // 显示远程分组界面
    void showGroupPanel(const QString &strGroup, bool isFocusOn);
    // 返回前一个页面
    void rebackPrevPanel();

public slots:
    // 处理连接远程操作
    void onItemClicked(const QString &key);
    // 处理焦点出列表的事件
    void onFocusOutList(Qt::FocusReason type);
    // 处理刷新列表信号
    void onRefreshList();

private:
    void initUI();
    void setSearchFilter(const QString &filter);

private:
    ListView *m_listWidget;
//    RemoteManagementPanelType m_previousPanel;  //用来保存调用当前搜索结果页的对象类型
    QString m_strGroupName;
    QString m_strFilter;
    // 判断是组内搜索还是组外搜索
    bool m_isGroupOrNot = false;
};

#endif  // REMOTEMANAGEMENTSEARCHPANEL_H
