// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "newdspinbox.h"
#include "utils.h"
#include <QDebug>
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#include <QRegExp>
#include <QRegExpValidator>
#else
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#endif

NewDspinBox::NewDspinBox(QWidget *parent) : DSpinBox(parent)
{
    // qDebug() << "NewDspinBox constructor start";
    Utils::set_Object_Name(this);

    setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    setButtonSymbols(QAbstractSpinBox::PlusMinus);
    
    // 设置默认范围（字体大小的范围）
    setRange(5, 50);
    
    // qDebug() << "NewDspinBox constructor end";
}

void NewDspinBox::setRange(int min, int max)
{
    // 设置数值范围
    this->setMinimum(min);
    this->setMaximum(max);
    
    // 更新验证器
    updateValidator(min, max);
}

void NewDspinBox::updateValidator(int min, int max)
{
    // 根据范围动态生成正则表达式
    QString regexPattern;
    
    if (min <= 9 && max <= 99) {
        // 对于字体大小 (5-50) 或类似的小范围
        regexPattern = QString("(^[%1-9]$)|(^[1-4][0-9]$)|(^[5][0]$)").arg(min);
    } else if (min >= 1000 && max <= 10000) {
        // 对于历史记录大小 (1000-10000) 
        // 匹配 1000-9999 和 10000
        regexPattern = "(^[1-9][0-9]{3}$)|(^10000$)";
    } else {
        // 对于其他范围，使用更通用的验证（只允许数字）
        int minDigits = QString::number(min).length();
        int maxDigits = QString::number(max).length();
        regexPattern = QString("^[0-9]{%1,%2}$").arg(minDigits).arg(maxDigits);
    }
    
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QRegExp regExp(regexPattern);
    lineEdit()->setValidator(new QRegExpValidator(regExp, this));
#else
    QRegularExpression regExp(regexPattern);
    lineEdit()->setValidator(new QRegularExpressionValidator(regExp, this));
#endif
}

void NewDspinBox::wheelEvent(QWheelEvent *event)
{
    // qDebug() << "NewDspinBox wheelEvent start";
    if (lineEdit()->hasFocus())
        DSpinBox::wheelEvent(event);
    // qDebug() << "NewDspinBox wheelEvent end";
}

