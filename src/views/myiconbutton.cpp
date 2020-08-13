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
#include "myiconbutton.h"

#include <DLog>
#include <utils.h>

MyIconButton::MyIconButton(QWidget *parent) : DIconButton(parent)
{
    Utils::set_Object_Name(this);
    //qDebug() << "MyIconButton-objectname" << objectName();
}

/*******************************************************************************
 1. @函数:    enterEvent
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    进入事件
*******************************************************************************/
void MyIconButton::enterEvent(QEvent *event)
{
    setIcon(QIcon(":/icons/deepin/builtin/focus/edit_press.svg"));
    DIconButton::enterEvent(event);
}

/*******************************************************************************
 1. @函数:    leaveEvent
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    离开事件
*******************************************************************************/
void MyIconButton::leaveEvent(QEvent *event)
{
    setIcon(QIcon(":/icons/deepin/builtin/hover/edit_hover.svg"));
    DIconButton::leaveEvent(event);
}
