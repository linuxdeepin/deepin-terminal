/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:  wangliang<wangliang@uniontech.com>
 *
 * Maintainer: wangliang<wangliang@uniontech.com>
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

#ifndef TABLETWINDOW_H
#define TABLETWINDOW_H

#include "mainwindow.h"

/*******************************************************************************
 1. @类名:    TabletMainWindow
 2. @作者:    ut000438 wangliang
 3. @日期:    2021-01-05
 4. @说明:    平板模式终端窗口
*******************************************************************************/
class TabletWindow : public MainWindow
{
    Q_OBJECT

public:
    ~TabletWindow() override;

    static TabletWindow *instance(TermProperties properties);

    /******** Add by ut001000 renfeixiang 2020-08-07:普通窗口不做处理***************/
    virtual void updateMinHeight() override {return;}

    // 虚拟键盘是否激活(即是否显示)的DBus信号
    void initVirtualKeyboardImActiveChangedSignal();

    // 虚拟键盘是否改变了geometry的DBus信号
    void initVirtualKeyboardGeometryChangedSignal();

    // 初始化虚拟键盘相关信号连接
    void initVirtualKeyboardConnections();

protected:
    // 初始化标题栏
    virtual void initTitleBar() override;
    // 初始化窗口属性
    virtual void initWindowAttribute() override;
    // 保存窗口尺寸
    virtual void saveWindowSize() override;
    // 切换全屏
    virtual void switchFullscreen(bool forceFullscreen = false) override;
    // 计算快捷预览显示坐标
    virtual QPoint calculateShortcutsPreviewPoint() override;
    // 处理雷神窗口丢失焦点自动隐藏功能
    virtual void onAppFocusChangeForQuake() override;
    // 根据字体和字体大小设置最小高度
    virtual void setWindowMinHeightForFont() override {return;}

protected:
    void changeEvent(QEvent *event) override;

    void resizeEvent(QResizeEvent *event) override;

signals:
    void onResizeWindowHeight(int windowHeight, bool isKeyboardShow);

private slots:
    // 判断虚拟键盘是否激活(即是否显示)的槽函数
    void slotVirtualKeyboardImActiveChanged(bool isShow);

    // 判断虚拟键盘布局(geometry)是否改变的槽函数
    void slotVirtualKeyboardGeometryChanged(QRect rect);

    // 根据虚拟键盘是否显示，调整主窗口高度
    void slotResizeWindowHeight(int windowHeight, bool isKeyboardShow);

private:
    explicit TabletWindow(TermProperties properties, QWidget *parent = nullptr);

    // 处理虚拟键盘显示/隐藏的函数
    void handleVirtualKeyboardShowHide(bool isShow);

    void resizePlugin();

    static TabletWindow *m_window;

    // 虚拟键盘是否显示
    bool m_isVirtualKeyboardShow = false;

    // 虚拟键盘高度
    int m_virtualKeyboardHeight = -1;
};

#endif  // TABLETWINDOW_H
