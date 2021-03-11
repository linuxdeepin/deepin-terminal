/*  Copyright (C) 2008 e_k (e_k@users.sourceforge.net)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef TOOLS_H
#define TOOLS_H

#include <QWidget>
#include <QVBoxLayout>
#include <QString>
#include <QStringList>

QString get_kb_layout_dir();
void add_custom_color_scheme_dir(const QString& custom_dir);
const QStringList get_color_schemes_dirs();

#endif
