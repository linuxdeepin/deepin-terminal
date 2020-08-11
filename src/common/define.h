/*
* Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
*
* Author:     wangpeili <wangpeili@uniontech.com>
* Maintainer: wangpeili <wangpeili@uniontech.com>
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DTNG_DEFINE_H
#define DTNG_DEFINE_H

#include <QtGlobal>
#include <QString>
/*
   TERMINALWIDGET_VERSION is (major << 16) + (minor << 8) + patch.
   can be used like #if (TERMINALWIDGET_VERSION >= QT_VERSION_CHECK(0, 7, 1))
*/
#define TERMINALWIDGET_VERSION      QT_VERSION_CHECK(TERMINALWIDGET_VERSION_MAJOR, TERMINALWIDGET_VERSION_MINOR, TERMINALWIDGET_VERSION_PATCH)
#define KEY_WIDGET_RUN   (Qt::Key_Enter)

const QString TERM_WIDGET_NAME = "Konsole::TerminalScreen";

#endif // DTNG_DEFINE_H
