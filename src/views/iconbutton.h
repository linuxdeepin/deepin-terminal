// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/*******************************************************************************
 1. @类名:    IconButton
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    图标控件
             对DIconButton的键盘事件进行重写
             1）支持上下左右功能键点击事件的信号传出
             2）支持回车键的点击事件信号传出
             这些信号可供别的控件进行相应的事件处理
*******************************************************************************/
#ifndef ICONBUTTON_H
#define ICONBUTTON_H
// dtk
#include <DIconButton>

// qt
#include <QKeyEvent>
#include <QFocusEvent>

DWIDGET_USE_NAMESPACE

class IconButton : public DIconButton
{
    Q_OBJECT
public:
    explicit IconButton(QWidget *parent = nullptr);

signals:
    // 焦点交给前一个
    void preFocus();
    // 焦点出
    void focusOut(Qt::FocusReason type);
    // 被键盘点击
    void keyPressClicked();

protected:
    /**
     * @brief 键盘事件处理右键点击事件
     * @author ut000610 戴正文
     * @param event 右键点击事件
     */
    void keyPressEvent(QKeyEvent *event) override;
    /**
     * @brief 焦点切出
     * @author ut000610 戴正文
     * @param event 焦点移出事件
     */
    void focusOutEvent(QFocusEvent *event) override;
};

#endif // ICONBUTTON_H
