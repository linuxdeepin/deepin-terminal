// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iconbutton.h"
#include "utils.h"

//qt
#include <QDebug>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(views)
IconButton::IconButton(QWidget *parent)
    : DIconButton(parent)
{
    // qCDebug(views) << "Enter IconButton::IconButton";
    Utils::set_Object_Name(this);
}

void IconButton::keyPressEvent(QKeyEvent *event)
{
    // qCDebug(views) << "IconButton::keyPressEvent() entered, key:" << event->key();

    // 不处理向右的事件
    switch (event->key()) {
    case Qt::Key_Right:
    case Qt::Key_Up:
    case Qt::Key_Down:
        qCDebug(views)  << "Ignoring up/down key event";
        event->ignore();
        break;
    case Qt::Key_Left:
        qCDebug(views)  << "Processing left key, emitting preFocus";
        emit preFocus();
        break;
    default:
        // qCDebug(views) << "Default key handling";
        DIconButton::keyPressEvent(event);
        break;
    }
}

void IconButton::focusOutEvent(QFocusEvent *event)
{
    // qCDebug(views) << event->reason() << "IconButton" << this;
    emit focusOut(event->reason());
    DIconButton::focusOutEvent(event);
}
