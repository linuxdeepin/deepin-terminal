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
#include "iconbutton.h"

//qt
#include <QDebug>

IconButton::IconButton(QWidget *parent)
    : DIconButton(parent)
{

}

/*******************************************************************************
 1. @函数:    keyPressEvent
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-17
 4. @说明:    处理右键点击事件
*******************************************************************************/
void IconButton::keyPressEvent(QKeyEvent *event)
{
    // 不处理向右的事件
    switch (event->key()) {
    case Qt::Key_Right:
    case Qt::Key_Up:
    case Qt::Key_Down:
        event->ignore();
        break;
    case Qt::Key_Left:
        emit preFocus();
        break;
    default:
        DIconButton::keyPressEvent(event);
        break;
    }
}

/*******************************************************************************
 1. @函数:    focusOutEvent
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-21
 4. @说明:    焦点出
*******************************************************************************/
void IconButton::focusOutEvent(QFocusEvent *event)
{
    qDebug() << event->reason() << "IconButton" << this;
    emit focusOut(event->reason());
    DIconButton::focusOutEvent(event);
}
