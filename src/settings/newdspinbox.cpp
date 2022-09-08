// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "newdspinbox.h"
#include "utils.h"

NewDspinBox::NewDspinBox(QWidget *parent) : DSpinBox(parent)
{
    Utils::set_Object_Name(this);

    setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    setButtonSymbols(QAbstractSpinBox::PlusMinus);
    /******** Add by nt001000 renfeixiang 2020-05-26 :增加正则表达式限制00000现象 Begin***************/
    QRegExp regExp("(^[1-4][0-9]$)|(^[5][0]$)|(^[1-9]$)");
    lineEdit()->setValidator(new QRegExpValidator(regExp, this));
    this->setMinimum(5);
    this->setMaximum(50);
    /******** Add by nt001000 renfeixiang 2020-05-26:增加正则表达式限制00000现象 End***************/
}

void NewDspinBox::wheelEvent(QWheelEvent *event)
{
    if (lineEdit()->hasFocus())
        DSpinBox::wheelEvent(event);
}

