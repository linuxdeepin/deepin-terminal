// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/*******************************************************************************
 1. @类名:    CustomCommandPanel
 2. @作者:    ut000125 孙成熙
 3. @日期:    2020-08-05
 4. @说明:    自定义命令面板类
*******************************************************************************/

#ifndef CUSTOMCOMMANDPANEL_H
#define CUSTOMCOMMANDPANEL_H

#include "rightpanel.h"
#include "customcommandsearchrstpanel.h"
#include "commonpanel.h"
#include "listview.h"

#include <DGroupBox>
#include <DSearchEdit>
#include <DListView>
#include <DButtonBox>
#include <DPushButton>

#include <QWidget>

DWIDGET_USE_NAMESPACE

class CustomCommandOptDlg;

class CustomCommandPanel : public CommonPanel
{
    Q_OBJECT
public:
    explicit CustomCommandPanel(QWidget *parent = nullptr);
    ~CustomCommandPanel();
    /**
     * @brief 自定义命令页面刷新
     * @author sunchengxi
     */
    void refreshCmdPanel();
    /**
     * @brief 自定义命令面板搜索框显示布局控制
     * @author sunchengxi
     */
    void refreshCmdSearchState();
    /**
     * @brief 将焦点设置到平面：若有搜索框，焦点进搜索框；若没搜索框，焦点进列表；若没列表，焦点进添加按钮
     * @author ut000610 戴正文
     */
    void setFocusInPanel();
signals:
    void handleCustomCurCommand(const QString &strCommand);
    void focusOut();
    void showSearchResult(const QString &strCommand);
public slots:
    /**
     * @brief 处理显示搜索结果的槽函数
     * @author sunchengxi
     */
    void showCurSearchResult();
    /**
     * @brief 增加自定义命令操作的槽函数
     * @author sunchengxi
     */
    void showAddCustomCommandDlg();
    /**
     * @brief 处理点击执行自定义命令
     * @author sunchengxi
     * @param key 自定义命令
     */
    void doCustomCommand(const QString &key);
    /**
     * @brief 处理焦点出列表的事件
     * @author ut000610 戴正文
     * @param type
     */
    void onFocusOut(Qt::FocusReason type);
    /**
     * @brief 添加自定义命令的响应
     * @param result:accept or reject
     */
    void onAddCommandResponse(int result);
private:
    /**
     * @brief 自定义命令面板界面初始化
     * @author sunchengxi
     */
    void initUI();

public://private:
    ListView *m_cmdListWidget = nullptr;
    CustomCommandOptDlg *m_pdlg = nullptr;
    QVBoxLayout *m_vlayout = nullptr;
    bool m_bpushButtonHaveFocus = false;
    DLabel *m_imageLabel = nullptr;
    DLabel *m_textLabel = nullptr;
    QVBoxLayout *m_backLayout = nullptr;
};

#endif  // CUSTOMCOMMANDPANEL_H
