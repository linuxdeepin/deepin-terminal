/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:  <wangpeili@uniontech.com>
 *
 * Maintainer:<wangpeili@uniontech.com>
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

#ifndef TERMINALAPPLICATION_H
#define TERMINALAPPLICATION_H

#include "environments.h"

#include <DApplication>

DWIDGET_USE_NAMESPACE

/*******************************************************************************
 1. @类名:    TerminalApplication
 2. @作者:    ut000439 王培利
 3. @日期:    2020-07-31
 4. @说明:    主要是为了拦截信号，调试部分功能使用
*******************************************************************************/
class TerminalApplication : public DApplication
{
    Q_OBJECT
public:
    TerminalApplication(int &argc, char *argv[]);
    ~TerminalApplication();
protected:
    // 重写了app.quit为当前窗口close.
    void handleQuitAction() override;
    bool notify(QObject *object, QEvent *event);
private:
};

#endif // TERMINALAPPLICATION_H
