/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:     sunchengxi <sunchengxi@uniontech.com>
 *
 * Maintainer: sunchengxi <sunchengxi@uniontech.com>
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
    void refreshCmdPanel();
    void refreshCmdSearchState();
    // 设置焦点进入平面
    void setFocusInPanel();
protected:
    void resizeEvent(QResizeEvent *event) override;
signals:
    void handleCustomCurCommand(const QString &strCommand);
    void focusOut();
    void showSearchResult(const QString &strCommand);
public slots:
    void showCurSearchResult();
    void showAddCustomCommandDlg();
    // 处理点击执行自定义命令
    void doCustomCommand(const QString &key);
    // 处理焦点出事件
    void onFocusOut(Qt::FocusReason type);

private:
    void initUI();

public://private:
    ListView *m_cmdListWidget = nullptr;
    CustomCommandOptDlg *m_pdlg = nullptr;
    QVBoxLayout *m_vlayout = nullptr;
    bool m_bpushButtonHaveFocus = false;

    //防止调用this->resize后循环触发resizeEvent
    bool m_isResizeBySelf = false;
};

#endif  // CUSTOMCOMMANDPANEL_H
