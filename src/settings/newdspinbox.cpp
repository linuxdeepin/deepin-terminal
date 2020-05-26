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

NewDspinBox::NewDspinBox(QWidget *parent) : DWidget(parent)
{
    m_DIconBtnAdd = new DIconButton(DStyle::SP_IncreaseElement);
    m_DIconBtnAdd->setFixedSize(36, 36);
    m_DIconBtnSubtract = new DIconButton(DStyle::SP_DecreaseElement);
    m_DIconBtnSubtract->setFixedSize(36, 36);
    m_DLineEdit = new DLineEdit();
    m_DLineEdit->setClearButtonEnabled(false);
    m_DLineEdit->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, false);
    QHBoxLayout *pHBoxLayout = new QHBoxLayout();

    pHBoxLayout->setSpacing(10);
    pHBoxLayout->setContentsMargins(0, 0, 0, 0);
    pHBoxLayout->addWidget(m_DLineEdit);
    pHBoxLayout->addWidget(m_DIconBtnAdd);
    pHBoxLayout->addWidget(m_DIconBtnSubtract);
    setLayout(pHBoxLayout);

//    m_QIntValidator = new QIntValidator(m_MinValue, m_MaxValue, this);
//    m_DLineEdit->lineEdit()->setValidator(m_QIntValidator);
//    m_DLineEdit->lineEdit()->setValidator(new QIntValidator(0, m_MaxValue, this));
    /******** Add by nt001000 renfeixiang 2020-05-26:增加正则表达式限制00000现象 Begin***************/
    QRegExp regExp("(^[1-4][0-9]$)|(^[5][0]$)|(^[5-9]$)");
    m_DLineEdit->lineEdit()->setValidator(new QRegExpValidator(regExp, this));
    /******** Add by nt001000 renfeixiang 2020-05-26:增加正则表达式限制00000现象 End***************/
    m_DLineEdit->setFocusPolicy(Qt::NoFocus);
    connect(m_DIconBtnAdd, &QAbstractButton::clicked, this, [ = ] {
        /***add by ut001121 zhangmeng 20200509 修复BUG#24848 设置中点击“+”“-”按钮修改字体大小，前方输入框未高亮***/
        m_DLineEdit->lineEdit()->setFocus();

        int value = m_DLineEdit->lineEdit()->text().toInt();
        if (value < m_MaxValue)
        {
            m_DLineEdit->lineEdit()->setText(QString::number(value + 1));
            emit valueChanged(m_DLineEdit->lineEdit()->text().toInt());
        }
    });
    connect(m_DIconBtnSubtract, &QAbstractButton::clicked, this, [ = ] {
        /***add by ut001121 zhangmeng 20200509 修复BUG#24848 设置中点击“+”“-”按钮修改字体大小，前方输入框未高亮***/
        m_DLineEdit->lineEdit()->setFocus();

        int value = m_DLineEdit->lineEdit()->text().toInt();
        if (value > m_MinValue)
        {
            m_DLineEdit->lineEdit()->setText(QString::number(value - 1));
            emit valueChanged(m_DLineEdit->lineEdit()->text().toInt());
        }
    });

    // 回车即脱离焦点
    connect(m_DLineEdit, &DLineEdit::returnPressed, this, [ = ] {

        m_DLineEdit->lineEdit()->clearFocus();
    });

    // 回车即脱离焦点
    connect(m_DLineEdit, &DLineEdit::editingFinished, this, [ = ] {
        m_DLineEdit->lineEdit()->clearFocus();
    });

    // 脱离焦点后校正生效．
    connect(m_DLineEdit, &DLineEdit::focusChanged, this, [ = ](bool var) {
        // 退出编辑的时候，数据做个校正
        if (!var) {
            /******** Add by nt001000 renfeixiang 2020-05-25:增加正常数据的过滤不需要处理，textChanged信号已经处理 Begin***************/
            int value = m_DLineEdit->lineEdit()->text().toInt();
            if(value >= 5)
                return ;
            /******** Add by nt001000 renfeixiang 2020-05-25:增加正常数据的过滤不需要处理，textChanged信号已经处理 End***************/
            correctValue();
            emit valueChanged(m_DLineEdit->lineEdit()->text().toInt());
        }
    });

    /******** Add by nt001000 renfeixiang 2020-05-25:增加m_DLineEdit的textChanged信号的响应 Begin***************/
    connect(m_DLineEdit, &DLineEdit::textChanged, this, [ = ](const QString &value) {
        //过滤范围之外的数据
        if(value.toInt() < 5)
            return ;
        // 对输入的数据做个校正
        //correctValue();
        emit valueChanged(m_DLineEdit->lineEdit()->text().toInt());
    });
    /******** Add by nt001000 renfeixiang 2020-05-25:增加m_DLineEdit的textChanged信号的响应 End***************/

    // 选择即进入
    connect(m_DLineEdit, &DLineEdit::selectionChanged, this, [ = ] {
        if (!m_DLineEdit->lineEdit()->hasFocus()
                && !m_DLineEdit->lineEdit()->selectedText().isEmpty())
        {
            m_DLineEdit->lineEdit()->setFocus();
        }
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

