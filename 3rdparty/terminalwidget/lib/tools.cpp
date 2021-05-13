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

#include "tools.h"

#include <QCoreApplication>
#include <QDir>
#include <QtDebug>
#include <QToolButton>


/*! Helper function to get possible location of layout files.
By default the KB_LAYOUT_DIR is used (linux/BSD/macports).
But in some cases (apple bundle) there can be more locations).
*/
QString get_kb_layout_dir()
{
//    qDebug() << __FILE__ << __FUNCTION__;

    QString rval = QString();
    QString k(QLatin1String(KB_LAYOUT_DIR));
    QDir d(k);

    //qDebug() << "default KB_LAYOUT_DIR: " << k;

    if (d.exists())
    {
        rval = k.append(QLatin1Char('/'));
        return rval;
    }

#ifdef Q_OS_MAC
    // subdir in the app location
    d.setPath(QCoreApplication::applicationDirPath() + QLatin1String("/kb-layouts/"));
    //qDebug() << d.path();
    if (d.exists())
        return QCoreApplication::applicationDirPath() + QLatin1String("/kb-layouts/");

    d.setPath(QCoreApplication::applicationDirPath() + QLatin1String("/../Resources/kb-layouts/"));
    if (d.exists())
        return QCoreApplication::applicationDirPath() + QLatin1String("/../Resources/kb-layouts/");
#endif
    //qDebug() << "Cannot find KB_LAYOUT_DIR. Default:" << k;
    return QString();
}

/*! Helper function to add custom location of color schemes.
*/
namespace {
    QStringList custom_color_schemes_dirs;
}
void add_custom_color_scheme_dir(const QString& custom_dir)
{
    if (!custom_color_schemes_dirs.contains(custom_dir))
        custom_color_schemes_dirs << custom_dir;
}

/*! Helper function to get possible locations of color schemes.
By default the COLORSCHEMES_DIR is used (linux/BSD/macports).
But in some cases (apple bundle) there can be more locations).
*/
const QStringList get_color_schemes_dirs()
{
//    qDebug() << __FILE__ << __FUNCTION__;

    QStringList rval;
    QString k(QLatin1String(COLORSCHEMES_DIR));
    QDir d(k);

//    qDebug() << "default COLORSCHEMES_DIR: " << k;

    if (d.exists())
        rval << k.append(QLatin1Char('/'));

#ifdef Q_OS_MAC
    // subdir in the app location
    d.setPath(QCoreApplication::applicationDirPath() + QLatin1String("/color-schemes/"));
    //qDebug() << d.path();
    if (d.exists())
    {
        if (!rval.isEmpty())
            rval.clear();
        rval << (QCoreApplication::applicationDirPath() + QLatin1String("/color-schemes/"));
    }
    d.setPath(QCoreApplication::applicationDirPath() + QLatin1String("/../Resources/color-schemes/"));
    if (d.exists())
    {
        if (!rval.isEmpty())
            rval.clear();
        rval << (QCoreApplication::applicationDirPath() + QLatin1String("/../Resources/color-schemes/"));
    }
#endif

    for (const QString& custom_dir : qAsConst(custom_color_schemes_dirs))
    {
        d.setPath(custom_dir);
        if (d.exists())
            rval << custom_dir;
    }
#ifdef QT_DEBUG
    if(!rval.isEmpty()) {
        qDebug() << "Using color-schemes: " << rval;
    } else {
        qDebug() << "Cannot find color-schemes in any location!";
    }
#endif
    return rval;
}
