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
#ifndef RIGHTPANEL_H
#define RIGHTPANEL_H

#include <QWidget>

/*******************************************************************************
 1. @类名:    RightPanel
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:
*******************************************************************************/

class RightPanel : public QWidget
{
    Q_OBJECT
public:
    explicit RightPanel(QWidget *parent = nullptr);

    /**
     * @brief 处理隐藏事件
     * @author ut001121 张猛
     * @param event 隐藏事件
     */
    void hideEvent(QHideEvent *event) override;
public slots:
    /**
     * @brief 动画显示
     * @author ut000610 daizhengwen
     */
    void showAnim();
    /**
     * @brief 动画隐藏
     * @author ut000610 daizhengwen
     */
    void hideAnim();

};

#endif  // RIGHTPANEL_H
