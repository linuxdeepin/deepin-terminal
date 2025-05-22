// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "newdspinbox.h"
#include "utils.h"
#include <QDebug>

NewDspinBox::NewDspinBox(QWidget *parent) : DSpinBox(parent)
{
    qDebug() << "NewDspinBox constructor start";
    Utils::set_Object_Name(this);

    setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    setButtonSymbols(QAbstractSpinBox::PlusMinus);
    /******** Add by nt001000 renfeixiang 2020-05-26 :增加正则表达式限制00000现象 Begin***************/
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QRegExp regExp("(^[1-4][0-9]$)|(^[5][0]$)|(^[1-9]$)");
    lineEdit()->setValidator(new QRegExpValidator(regExp, this));
#else
    QRegularExpression regExp("(^[1-4][0-9]$)|(^[5][0]$)|(^[1-9]$)");
    lineEdit()->setValidator(new QRegularExpressionValidator(regExp, this));
#endif
    this->setMinimum(5);
    this->setMaximum(50);
    /******** Add by nt001000 renfeixiang 2020-05-26:增加正则表达式限制00000现象 End***************/
    qDebug() << "NewDspinBox constructor end";
}

void NewDspinBox::wheelEvent(QWheelEvent *event)
{
    qDebug() << "NewDspinBox wheelEvent start";
    if (lineEdit()->hasFocus())
        DSpinBox::wheelEvent(event);
    qDebug() << "NewDspinBox wheelEvent end";
}

