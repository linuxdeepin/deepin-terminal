/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:  wangpeili<wangpeili@uniontech.com>
 *
 * Maintainer:wangpeili<wangpeili@uniontech.com>
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
#include "termproperties.h"

TermProperties::TermProperties(QString workingDir)
{
    m_properties.insert(WorkingDir, workingDir);
}

TermProperties::TermProperties(QString workingDir, QString colorScheme)
{
    m_properties.insert(WorkingDir, workingDir);
    m_properties.insert(ColorScheme, colorScheme);
}

TermProperties::TermProperties(QMap<TermProperty, QVariant> list) : m_properties(list)
{
}

/*******************************************************************************
 1. @函数:    setWorkingDir
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    设置工作目录
*******************************************************************************/
void TermProperties::setWorkingDir(QString workingDir)
{
    m_properties.insert(WorkingDir, workingDir);
}

/*******************************************************************************
 1. @函数:    setTermPropertyMap
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    设置终端属性图
*******************************************************************************/
void TermProperties::setTermPropertyMap(QMap<TermProperty, QVariant> list)
{
    m_properties = list;
}

/*******************************************************************************
 1. @函数:    contains
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    判断终端属性中是否包含参数属性
*******************************************************************************/
bool TermProperties::contains(TermProperty propertyType) const
{
    return m_properties.contains(propertyType);
}

/*******************************************************************************
 1. @函数:    operator[]
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    重载[]操作符
*******************************************************************************/
QVariant &TermProperties::operator[](const TermProperty &key)
{
    return m_properties[key];
}

/*******************************************************************************
 1. @函数:    operator[]
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    重载[]操作符
*******************************************************************************/
const QVariant TermProperties::operator[](const TermProperty &key) const
{
    return m_properties[key];
}
