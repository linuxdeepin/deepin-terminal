/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     rekols <rekols@foxmail.com>
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

#ifndef UTILS_H
#define UTILS_H
#include "settings.h"
#include "termproperties.h"
#include <DPushButton>
#include <DApplication>

#include <QObject>
#include <QHash>
#include <QFont>
#include <QList>
#include <QVariant>
#include <QCommandLineParser>

/******** Modify by m000714 daizhengwen 2020-04-03: 自绘的显示控件限制item的宽度****************/
#define ITEMMAXWIDTH 160
/********************* Modify by m000714 daizhengwen End ************************/

/******** Modify by m000714 daizhengwen 2020-04-17: 使用DTK的Dialog****************/
#define USE_DTK
/********************* Modify by m000714 daizhengwen End ************************/

// kwin dbus
#define KWinDBusService "org.kde.KWin"
#define KWinDBusPath "/KWin"

class Utils : public QObject
{
    Q_OBJECT

public:
    Utils(QObject *parent = nullptr);
    ~Utils();
    enum CloseType {
        CloseType_Window,
        CloseType_Tab,
        CloseType_OtherTab,
        CloseType_Terminal,
        CloseType_OtherTerminals,
    };

    static QHash<QString, QPixmap> m_imgCacheHash;
    static QHash<QString, QString> m_fontNameCache;

    static QString getQssContent(const QString &filePath);
    static QString getConfigPath();
    static QString suffixList();
    static QPixmap renderSVG(const QString &filePath, const QSize &size);
    static QString loadFontFamilyFromFiles(const QString &fontFileName);
    static QString getElidedText(QFont font, QString str, int MaxWith);

    static const QString holdTextInRect(const QFont &font, QString text, const QSize &size);

    static QString convertToPreviewString(QString fontFilePath, QString srcString);

    static QString getRandString();

    static QString showDirDialog(QWidget *widget);
    static QStringList showFilesSelectDialog(QWidget *widget);

    static bool showExitConfirmDialog(CloseType type, int count = 1);
    static void getExitDialogText(CloseType type, QString &title, QString &txt, int count = 1);
    static bool showExitUninstallConfirmDialog();
    static bool showUnistallConfirmDialog();
    static bool showShortcutConflictDialog(QString conflictkey);
    static bool showShortcutConflictMsgbox(QString txt);
    static void setSpaceInWord(DPushButton *button);

    static void showRenameTitleDialog(QString oldTitle, QWidget *parentWidget);

    static void showSameNameDialog(QWidget *parent, const QString &firstLine, const QString &secondLine);
    static void clearChildrenFocus(QObject *objParent);

    static QCommandLineParser &setCommandLineParser(QString appDesc, DApplication &app, QCommandLineParser &parser);
    static TermProperties parseArgument( QStringList arguments);

};

#endif
