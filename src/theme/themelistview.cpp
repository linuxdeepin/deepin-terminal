/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:     wangpeili <wangpeili@uniontech.com>
 *
 * Maintainer: wangpeili <wangpeili@uniontech.com>
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
#include "themelistview.h"
#include "utils.h"
#include <QDebug>

ThemeListView::ThemeListView(QWidget *parent) : QListView(parent)
{
    Utils::set_Object_Name(this);
    setVerticalScrollMode(ScrollPerPixel);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

/*******************************************************************************
 1. @函数:    focusOutEvent
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-12
 4. @说明:    焦点丢失事件
*******************************************************************************/
void ThemeListView::focusOutEvent(QFocusEvent *event)
{
    emit focusOut();

    QListView::focusOutEvent(event);
}

/*******************************************************************************
 1. @函数:    selectionChanged
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-12
 4. @说明:    选择已更改
*******************************************************************************/
void ThemeListView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    // Theme changed.
    QModelIndexList list = selected.indexes();
    const QModelIndex &index = list.first();
    const QString &themeName = index.data(1919810).toString();
    emit themeChanged(themeName);
//    for (const QModelIndex &index : list) {
//        const QString &themeName = index.data(1919810).toString();
//        emit themeChanged(themeName);
//        break;
//    }

    QListView::selectionChanged(selected, deselected);
}
