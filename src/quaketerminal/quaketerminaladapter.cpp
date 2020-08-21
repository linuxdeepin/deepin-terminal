/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:  wangliang<wangliang@uniontech.com>
 *
 * Maintainer:wangliang<wangliang@uniontech.com>
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
#include "quaketerminaladapter.h"

#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QMetaObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include "utils.h"

/*
 * Implementation of adaptor class QuakeTerminalAdapter
 */

QuakeTerminalAdapter::QuakeTerminalAdapter(QObject *parent) : QDBusAbstractAdaptor(parent)
{
    Utils::set_Object_Name(this);
    // constructor
    setAutoRelaySignals(true);
}

QuakeTerminalAdapter::~QuakeTerminalAdapter()
{
    // destructor
}

/*******************************************************************************
 1. @函数:    ShowOrHide
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-12
 4. @说明:    显示或隐藏
*******************************************************************************/
void QuakeTerminalAdapter::ShowOrHide()
{
    qDebug() << "QuakeTerminalAdapter::ShowOrHide";
    QMetaObject::invokeMethod(parent(), "ShowOrHide");
}
