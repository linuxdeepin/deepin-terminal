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
#include "utils.h"

//qt
#include <QDebug>

IconButton::IconButton(QWidget *parent)
    : DIconButton(parent)
{
    Utils::set_Object_Name(this);
}

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

void IconButton::focusOutEvent(QFocusEvent *event)
{
    qInfo() << event->reason() << "IconButton" << this;
    emit focusOut(event->reason());
    DIconButton::focusOutEvent(event);
}
