// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iconbutton.h"
#include "utils.h"

//qt
#include <QDebug>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(LogViews)
IconButton::IconButton(QWidget *parent)
    : DIconButton(parent)
{
    Utils::set_Object_Name(this);
}

void IconButton::keyPressEvent(QKeyEvent *event)
{
    // 不处理向右的事件
    switch (event->key()) {
    case Qt::Key_Right:
    case Qt::Key_Up:
    case Qt::Key_Down:
        event->ignore();
        break;
    case Qt::Key_Left:
        emit preFocus();
        break;
    default:
        DIconButton::keyPressEvent(event);
        break;
    }
}

void IconButton::focusOutEvent(QFocusEvent *event)
{
    qCInfo(LogViews) << event->reason() << "IconButton" << this;
    emit focusOut(event->reason());
    DIconButton::focusOutEvent(event);
}
