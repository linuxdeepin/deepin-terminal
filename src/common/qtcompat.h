// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QTCOMPAT_H
#define QTCOMPAT_H

#include <QtGlobal>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QRegularExpression>
class QEnterEvent;

#define SKIP_EMPTY_PARTS    Qt::SkipEmptyParts
#define ENDL                Qt::endl
#define REG_EXP             QRegularExpression
#define REG_EXPV            QRegularExpressionValidator

using EnterEvent = QEnterEvent;

#else // QT_VERSION < 6.0.0

#include <QRegExp>
class QEvent;

#define SKIP_EMPTY_PARTS    QString::SkipEmptyParts
#define ENDL                endl
#define REG_EXP             QRegExp
#define REG_EXPV            QRegExpValidator

using EnterEvent = QEvent;

#endif

#endif // QTCOMPAT_H
