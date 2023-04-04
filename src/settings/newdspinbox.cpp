// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "newdspinbox.h"
#include "utils.h"

NewDspinBox::NewDspinBox(QWidget *parent) : DSpinBox(parent)
{
    Utils::set_Object_Name(this);

    setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    setButtonSymbols(QAbstractSpinBox::PlusMinus);
}

void NewDspinBox::wheelEvent(QWheelEvent *event)
{
    if (lineEdit()->hasFocus())
        DSpinBox::wheelEvent(event);
}

