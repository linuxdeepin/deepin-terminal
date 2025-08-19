// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "newdspinbox.h"
#include "utils.h"
#include <QDebug>
#include <QFocusEvent>
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#include <QRegExp>
#include <QRegExpValidator>
#else
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#endif

NewDspinBox::NewDspinBox(QWidget *parent) : DSpinBox(parent), m_minValue(5), m_maxValue(50)
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
    // 保存范围信息
    m_minValue = min;
    m_maxValue = max;
    
    // 对于历史记录大小，允许临时输入更小的值以支持智能补全
    if (min >= 1000 && max <= 10000) {
        // 设置更宽的范围：1-10000，允许用户输入小值进行智能补全
        this->setMinimum(1);
        this->setMaximum(max);
    } else {
        // 对于其他情况（如字体大小），使用原始范围
        this->setMinimum(min);
        this->setMaximum(max);
    }
    
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
        // 允许输入1-4位数字，支持智能补全: 1-9, 10-99, 100-999, 1000-10000
        regexPattern = "(^[1-9]$)|(^[1-9][0-9]$)|(^[1-9][0-9]{2}$)|(^[1-9][0-9]{3}$)|(^10000$)";
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

void NewDspinBox::focusOutEvent(QFocusEvent *event)
{
    // 在失去焦点时执行智能补全
    if (m_minValue >= 1000 && m_maxValue <= 10000) {
        int completedValue = getSmartCompleteFromText();
        if (completedValue >= m_minValue && completedValue <= m_maxValue) {
            setValue(completedValue);
        }
    }
    
    DSpinBox::focusOutEvent(event);
}

void NewDspinBox::keyPressEvent(QKeyEvent *event)
{
    // 处理回车键和Enter键的智能补全
    if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) 
        && m_minValue >= 1000 && m_maxValue <= 10000) {
        
        int completedValue = getSmartCompleteFromText();
        if (completedValue >= m_minValue && completedValue <= m_maxValue) {
            setValue(completedValue);
            // 选中所有文本，方便用户查看补全结果
            lineEdit()->selectAll();
            return;
        }
    }
    
    DSpinBox::keyPressEvent(event);
}

int NewDspinBox::smartComplete(int value)
{
    // 只对历史记录大小范围进行智能补全
    if (m_minValue < 1000 || m_maxValue > 10000) {
        return value;
    }
    
    // 如果已经是有效范围内的完整值，不需要补全
    if (value >= 1000 && value <= 10000) {
        return value;
    }
    
    // 智能补全逻辑
    if (value >= 1 && value <= 9) {
        // 1-9 → 1000-9000 (如: 1→1000, 2→2000, 5→5000)
        return value * 1000;
    } else if (value >= 10 && value <= 99) {
        // 10-99 → 1000-9900 (如: 11→1100, 22→2200, 55→5500)
        return value * 100;
    } else if (value >= 100 && value <= 999) {
        // 100-999 → 1000-9990 (如: 110→1100, 550→5500)
        return value * 10;
    }
    
    // 如果不在预期范围内，返回默认值
    return 5000;
}

int NewDspinBox::getSmartCompleteFromText()
{
    // 获取输入框中的文本
    QString text = lineEdit()->text().trimmed();
    
    // 尝试转换为整数
    bool ok;
    int inputValue = text.toInt(&ok);
    
    if (!ok || inputValue <= 0) {
        // 无效输入，返回当前值
        return value();
    }
    
    // 使用智能补全逻辑
    return smartComplete(inputValue);
}

void NewDspinBox::wheelEvent(QWheelEvent *event)
{
    // qDebug() << "NewDspinBox wheelEvent start";
    if (lineEdit()->hasFocus())
        DSpinBox::wheelEvent(event);
    // qDebug() << "NewDspinBox wheelEvent end";
}

