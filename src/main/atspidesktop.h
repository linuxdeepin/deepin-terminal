/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:  <daizhengwen@uniontech.com>
 *
 * Maintainer:<daizhengwen@uniontech.com>
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
#ifndef ATSPIDESKTOP_H
#define ATSPIDESKTOP_H
// qt
#include <QThread>

// lib
#include <atspi/atspi.h>


/*******************************************************************************
 1. @类名:    AtspiDesktop
 2. @作者:    戴正文
 3. @日期:    2020-07-31
 4. @说明:    该类负责初始化qt-at-spi
             无障碍辅助工具
             相关链接 : https://github.com/infapi00/at-spi2-examples/blob/master/c/notify-value-changes.c
             qt-at-spi 快捷键映射依赖此库 让Qt可以识别ctrl + shift + ?快捷键
*******************************************************************************/

class AtspiDesktop : public QThread
{
    Q_OBJECT
public:
    AtspiDesktop();
    void stopThread();

    // 事件负责处理g_object
    static void on_event(AtspiEvent *event, void *data);

protected:
    void run() override;

private:
    // 是否进入循环
    bool m_isLoop = false;
};

#endif // ATSPIDESKTOP_H
