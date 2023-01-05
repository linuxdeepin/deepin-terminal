// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
