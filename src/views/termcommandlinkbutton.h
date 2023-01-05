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
#ifndef TERMCOMMANDLINKBUTTON_H
#define TERMCOMMANDLINKBUTTON_H

#include <DPushButton>

DWIDGET_USE_NAMESPACE

/*******************************************************************************
 1. @类名:    TermCommandLinkButton
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    重写DPushButton
*******************************************************************************/

class TermCommandLinkButton : public DPushButton
{
    Q_OBJECT
public:
    /**
     * @brief 设置DPushButton属性，字体颜色，焦点， 提高边框
     * @author ut000610 daizhengwen
     * @param parent
     */
    explicit TermCommandLinkButton(QWidget *parent = nullptr);
};

#endif // TERMCOMMANDLINKBUTTON_H
