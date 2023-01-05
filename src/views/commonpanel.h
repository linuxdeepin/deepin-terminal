/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:     daizhengwen <daizhengwen@uniontech.com>
 *
 * Maintainer: daizhengwen <daizhengwen@uniontech.com>
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
#ifndef COMMONPANEL_H
#define COMMONPANEL_H

#include "rightpanel.h"
#include "iconbutton.h"

#include <DPushButton>
#include <DIconButton>
#include <DSearchEdit>
#include <DListWidget>
#include <DLabel>

#include <QFrame>

DWIDGET_USE_NAMESPACE

/*******************************************************************************
 1. @类名:    CommonPanel
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:
*******************************************************************************/

class CommonPanel : public QFrame
{
    Q_OBJECT
public:
    explicit CommonPanel(QWidget *parent = nullptr);
    /**
     * @brief 清除搜索信息
     * @author ut000610 daizhengwen
     */
    void clearSearchInfo();

public slots:
    /**
     * @brief 回车后，焦点进入返回键
     * @author ut000610 戴正文
     */
    void onFocusInBackButton();

signals:
    void focusOut();

public:
    DIconButton *m_backButton = nullptr;
    IconButton *m_rebackButton = nullptr;
    DSearchEdit *m_searchEdit = nullptr;
    DPushButton *m_pushButton = nullptr;
    DLabel *m_label = nullptr;
    bool m_isShow = false;
};

#endif  // COMMONPANEL_H
