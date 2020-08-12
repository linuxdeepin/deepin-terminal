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
#ifndef QUAKETERMINALADAPTER_H
#define QUAKETERMINALADAPTER_H

#include <QObject>
#include <QtDBus>

#define kQuakeTerminalService "com.deepin.quake_terminal"
#define kQuakeTerminalIface "/com/deepin/quake_terminal"

/*
 * Adaptor class for interface com.deepin.quake_terminal
 */

/*******************************************************************************
 1. @类名:    QuakeTerminalAdapter
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-12
 4. @说明:
*******************************************************************************/

class QuakeTerminalAdapter : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.quake_terminal")
    Q_CLASSINFO("D-Bus Introspection", ""
                                       "  <interface name=\"com.deepin.quake_terminal\">\n"
                                       "    <method name=\"ShowOrHide\">\n"
                                       "    </method>\n"
                                       "  </interface>\n"
                                       "")

public:
    QuakeTerminalAdapter(QObject *parent);
    virtual ~QuakeTerminalAdapter();

public Q_SLOTS:
    void ShowOrHide();
};

#endif  // QUAKETERMINALADAPTER_H
