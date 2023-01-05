/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:     daizhengwen <daizhengwen@uniontech.com>
 *
 * Maintainer: daizhengwen <daizhengwen@uniontech.com>
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
