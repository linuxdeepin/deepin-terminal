/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:     wangliang <wangliang@uniontech.com>
 *
 * Maintainer: wangliang <wangliang@uniontech.com>
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

#ifndef TERMINALWIDGET_EXPORT_H
#define TERMINALWIDGET_EXPORT_H

#ifdef TERMINALWIDGET5_STATIC_DEFINE
#  define TERMINALWIDGET_EXPORT
#  define TERMINALWIDGET5_NO_EXPORT
#else
#  ifndef TERMINALWIDGET_EXPORT
#    ifdef terminalwidget5_EXPORTS
        /* We are building this library */
#      define TERMINALWIDGET_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define TERMINALWIDGET_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef TERMINALWIDGET5_NO_EXPORT
#    define TERMINALWIDGET5_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef TERMINALWIDGET5_DEPRECATED
#  define TERMINALWIDGET5_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef TERMINALWIDGET5_DEPRECATED_EXPORT
#  define TERMINALWIDGET5_DEPRECATED_EXPORT TERMINALWIDGET_EXPORT TERMINALWIDGET5_DEPRECATED
#endif

#ifndef TERMINALWIDGET5_DEPRECATED_NO_EXPORT
#  define TERMINALWIDGET5_DEPRECATED_NO_EXPORT TERMINALWIDGET5_NO_EXPORT TERMINALWIDGET5_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef TERMINALWIDGET5_NO_DEPRECATED
#    define TERMINALWIDGET5_NO_DEPRECATED
#  endif
#endif

#endif /* TERMINALWIDGET_EXPORT_H */
