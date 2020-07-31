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
 1. @类名:    CustomCommandItemModel
 2. @作者:    ut000125 孙成熙
 3. @日期:    2020-07-31
 4. @说明:    代理数据模型
             数据模型的初始化
             数据模型的增加
*******************************************************************************/

#ifndef CUSTOMCOMMANDITEMMODEL_H
#define CUSTOMCOMMANDITEMMODEL_H

#include <QSortFilterProxyModel>
#include <QAction>

typedef struct {
    QString m_cmdName;
    QString m_cmdText;
    QString m_cmdShortcut;
} CustomCommandItemData;

Q_DECLARE_METATYPE(CustomCommandItemData)

class CustomCommandItemModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    CustomCommandItemModel(QObject *parent = nullptr);

    void initCommandListData(const QList<CustomCommandItemData> &cmdListData);
    void addNewCommandData(const CustomCommandItemData itemData);
};

#endif // CUSTOMCOMMANDITEMMODEL_H
