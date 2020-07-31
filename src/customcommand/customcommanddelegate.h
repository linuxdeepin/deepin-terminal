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
 1. @类名:    CustomCommandDelegate
 2. @作者:    ut000125 孙成熙
 3. @日期:    2020-07-31
 4. @说明:    自定义列表的委托代理类
             自绘列表项
*******************************************************************************/

#ifndef CUSTOMCOMMANDDELEGATE_H
#define CUSTOMCOMMANDDELEGATE_H

#include <DStyledItemDelegate>

DWIDGET_USE_NAMESPACE

class CustomCommandDelegate : public DStyledItemDelegate
{
public:
    explicit CustomCommandDelegate(QAbstractItemView *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const Q_DECL_OVERRIDE;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const Q_DECL_OVERRIDE;

public:
    bool m_bModifyCheck = false;    //自定义命令列表项是否被选中
    bool m_bMouseOpt = false;       //是否是鼠标操作

private:
    QAbstractItemView *m_parentView = nullptr;
};

#endif // CUSTOMCOMMANDDELEGATE_H
