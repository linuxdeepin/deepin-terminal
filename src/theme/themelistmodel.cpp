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

ThemeListModel::ThemeListModel(QObject *parent) : QAbstractListModel(parent)
{
    initThemeData();
}

int ThemeListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_themeData.count();
}

QVariant ThemeListModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(role);
    const int row = index.row();
    return m_themeData[row];
}

void ThemeListModel::initThemeData()
{
    m_themeData = QTermWidget::availableColorSchemes();
}
