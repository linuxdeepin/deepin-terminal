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
#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QWidget>
#include <QHBoxLayout>

/*******************************************************************************
 1. @类名:    TitleBar
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-12
 4. @说明:
*******************************************************************************/

class TitleBar : public QWidget
{
    Q_OBJECT
public:
    TitleBar(QWidget *parent = nullptr);
    ~TitleBar();

    /**
     * @brief 设置标签栏
     * @author ut000610 daizhengwen
     * @param widget
     */
    void setTabBar(QWidget *widget);
    /**
     * @brief 右边的空间
     * @author ut000610 daizhengwen
     * @return
     */
    int rightSpace();

    /**
     * @brief setVerResized 设置是否手动resize当前界面
     * @param resized
     */
    void setVerResized(bool resized);
private:
    QHBoxLayout *m_layout = nullptr;
    int m_rightSpace;

    //自定义窗口resize bug#98888
private:
    int m_verResizedCurOff = 0;//当前垂直偏移量
    bool m_verResizedEnabled = false;//是否手动resize当前界面
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
};

#endif  // TITLEBAR_H
