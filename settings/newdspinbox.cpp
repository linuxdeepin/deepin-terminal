/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2011 ~ 2018 Deepin, Inc.
 *
 * Author:     Wang Peili <wangpeili@deepin.com>
 * Maintainer: Rekols     <rekols@foxmail.com>
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

#include "newdspinbox.h"
#include <DLog>

NewDspinBox::NewDspinBox(QWidget *parent) : QWidget(parent)
{
    m_DIconBtnAdd = new DIconButton(DStyle::SP_IncreaseElement);
    m_DIconBtnAdd->setFixedSize(36, 36);
    m_DIconBtnSubtract = new DIconButton(DStyle::SP_DecreaseElement);
    m_DIconBtnSubtract->setFixedSize(36, 36);
    m_DLineEdit = new DLineEdit();
    m_DLineEdit->setFixedSize(208, 36);
    m_DLineEdit->setClearButtonEnabled(false);
    QHBoxLayout *pHBoxLayout = new QHBoxLayout();

    pHBoxLayout->setSpacing(10);
    pHBoxLayout->setContentsMargins(0, 0, 0, 0);
    pHBoxLayout->addWidget(m_DLineEdit);
    pHBoxLayout->addWidget(m_DIconBtnAdd);
    pHBoxLayout->addWidget(m_DIconBtnSubtract);
    pHBoxLayout->addStretch();
    setLayout(pHBoxLayout);

    m_QIntValidator = new QIntValidator(m_MinValue, m_MaxValue, this);
    m_DLineEdit->lineEdit()->setValidator(m_QIntValidator);
    m_DLineEdit->lineEdit()->setValidator(new QIntValidator(0, m_MaxValue, this));
    m_DLineEdit->setFocusPolicy(Qt::NoFocus);
    connect(m_DIconBtnAdd, &QAbstractButton::clicked, this, [ = ] {
        int value = m_DLineEdit->lineEdit()->text().toInt();
        if (value < m_MaxValue)
        {
            m_DLineEdit->lineEdit()->setText(QString::number(value + 1));
            emit valueChanged(m_DLineEdit->lineEdit()->text().toInt());
        }
    });
    connect(m_DIconBtnSubtract, &QAbstractButton::clicked, this, [ = ] {
        int value = m_DLineEdit->lineEdit()->text().toInt();
        if (value > m_MinValue)
        {
            m_DLineEdit->lineEdit()->setText(QString::number(value - 1));
            emit valueChanged(m_DLineEdit->lineEdit()->text().toInt());
        }
    });
    connect(m_DLineEdit, &DLineEdit::focusChanged, this, [ = ](bool var) {
        // 退出编辑的时候，数据做个校正
        if (!var) {
            correctValue();
            emit valueChanged(m_DLineEdit->lineEdit()->text().toInt());
        }
    });
    connect(m_DLineEdit, &DLineEdit::returnPressed, this, [ = ] {
        // 不符合范围的数字不发信号出去
        m_DLineEdit->setFocus();
    });
    connect(m_DLineEdit, &DLineEdit::editingFinished, this, [ = ] {
        // 不符合范围的数字不发信号出去
        m_DLineEdit->setFocus();
    });
}

void NewDspinBox::setValue(int val)
{
    m_DLineEdit->setText(QString::number(val));
}

void NewDspinBox::setMaximum(int val)
{
    if (val > m_MinValue) {
        m_MaxValue = val;
        // m_QIntValidator->setTop(m_MaxValue);
        // m_DLineEdit->lineEdit()->setValidator(m_QIntValidator);
    }
}

void NewDspinBox::setMinimum(int val)
{
    if (val < m_MaxValue) {
        m_MinValue = val;
        // m_QIntValidator->setBottom(m_MinValue);
        // m_DLineEdit->lineEdit()->setValidator(m_QIntValidator);
    }
}
void NewDspinBox::correctValue()
{
    int val = m_DLineEdit->text().toInt();
    if (val > m_MaxValue) {
        m_DLineEdit->lineEdit()->setText(QString::number(m_MaxValue));
    }
    if (val < m_MinValue) {
        m_DLineEdit->lineEdit()->setText(QString::number(m_MinValue));
    }
}
