// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NEWDSPINBOX_H
#define NEWDSPINBOX_H

#include <DSpinBox>

#include <QRegExpValidator>
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

protected:
    /**
     * @brief 重写wheelEvent函数，只有有焦点的情况，才进行字体大小变化
     * @author ut001000 任飞翔
     * @param event
     */
    void wheelEvent(QWheelEvent *event) override;
};



#endif
