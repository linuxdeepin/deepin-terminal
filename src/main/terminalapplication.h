// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TERMINALAPPLICATION_H
#define TERMINALAPPLICATION_H

#include "environments.h"

//dtk
#include <DApplication>
#include <DPushButton>
#include <DKeySequenceEdit>

//qt
#include <QList>

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
    // 为了更精确的测算时间，单独设置
    /**
     * @brief 把main初始的时间，设置为应用启动时间
     * @author ut000439 王培利
     * @param time 时间
     */
    void setStartTime(qint64 time);
    /**
     * @brief 只有主进程在创建mainwindow的时候会用到。
     * @author ut000439 王培利
     * @return
     */
    qint64 getStartTime();
protected:
    // 重写了app.quit为当前窗口close.
    /**
     * @brief  重写了app.quit为当前窗口close.
     * @author ut000439 wangpeili
     */
    void handleQuitAction() override;
    /**
     * @brief 该部分代码，大都 用于调试，待整理
     * @author ut000439 wangpeili
     * @param object
     * @param event
     * @return
     */
    bool notify(QObject *object, QEvent *event);
private:
    /**
     * @brief 模拟键盘space按压效果
     * @author ut000610 戴正文
     * @param pushButton
     */
    void pressSpace(QObject *obj);

    // APP启动时间
    // 子进程该数据时间会通过共享内存传给对应的maindow
    qint64 m_AppStartTime = 0;
    // 记录已经进入编辑状态的DKeySequenceEdit
    QList<DKeySequenceEdit *> m_keySequenceList;
};

#endif // TERMINALAPPLICATION_H
