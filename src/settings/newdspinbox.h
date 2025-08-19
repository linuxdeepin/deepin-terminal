// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NEWDSPINBOX_H
#define NEWDSPINBOX_H

#include <DSpinBox>

// #include <QRegExpValidator>
#include <QKeyEvent>

DWIDGET_USE_NAMESPACE

/*******************************************************************************
 1. @类名:    NewDspinBox
 2. @作者:    ut000439 王培利
 3. @日期:    2020-07-31
 4. @说明:    设置里面重新写的spinBox 2020-11-06 renfeixiang 修改成继承DSpinBox 修改bug#53551
*******************************************************************************/
class NewDspinBox : public DSpinBox
{
    Q_OBJECT
public:
    explicit NewDspinBox(QWidget *parent = nullptr);
    /**
     * @brief 设置数值范围并更新验证器
     * @param min 最小值
     * @param max 最大值
     */
    void setRange(int min, int max);

protected:
    /**
     * @brief 重写wheelEvent函数，只有有焦点的情况，才进行字体大小变化
     * @author ut001000 任飞翔
     * @param event
     */
    void wheelEvent(QWheelEvent *event) override;

private:
    /**
     * @brief 根据最小值和最大值创建合适的正则表达式验证器
     * @param min 最小值
     * @param max 最大值
     */
    void updateValidator(int min, int max);
};



#endif
