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

private:
    explicit TabletWindow(TermProperties properties, QWidget *parent = nullptr);
    static TabletWindow *m_window;
};

#endif  // TABLETWINDOW_H
