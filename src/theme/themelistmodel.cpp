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
#include "themelistmodel.h"

#include "qtermwidget.h"
#include "utils.h"
#include <QDebug>

ThemeListModel::ThemeListModel(QObject *parent) : QAbstractListModel(parent)
{
    Utils::set_Object_Name(this);
    initThemeData();
}

/*******************************************************************************
 1. @函数:    rowCount
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-12
 4. @说明:    行数
*******************************************************************************/
int ThemeListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_themeData.count();
}

/*******************************************************************************
 1. @函数:    data
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-12
 4. @说明:    获取数据
*******************************************************************************/
QVariant ThemeListModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(role);
    const int row = index.row();
    return m_themeData[row];
}

/*******************************************************************************
 1. @函数:    initThemeData
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-12
 4. @说明:    初始化主题数据
*******************************************************************************/
void ThemeListModel::initThemeData()
{
    m_themeData = QTermWidget::availableColorSchemes();
}
