// Copyright (C) 2019 ~ 2023 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUSMANAGER_H
#define DBUSMANAGER_H
#include "service.h"

#include <QObject>
#include <QList>
#include <QVariant>

// deepin-terminal
#define TERMINALSERVER "com.deepin.terminal.reborn"
#define TERMINALINTERFACE "/window"

// kwin dbus
#define KWINDBUSSERVICE "org.kde.KWin"
#define KWINDBUSPATH "/KWin"

// sound effect 音效服务
//#define SOUND_EFFECT_SERVICE    "com.deepin.daemon.SoundEffect"
//#define SOUND_EFFECT_PATH       "/com/deepin/daemon/SoundEffect"
//#define SOUND_EFFECT_INTERFACE  "com.deepin.daemon.SoundEffect"
#define SOUND_EFFECT_SERVICE    "org.deepin.dde.SoundEffect1"
#define SOUND_EFFECT_PATH       "/org/deepin/dde/SoundEffect1"
#define SOUND_EFFECT_INTERFACE  "org.deepin.dde.SoundEffect1"
#define SOUND_EFFECT_METHOD(method) QDBusMessage::createMethodCall(SOUND_EFFECT_SERVICE, SOUND_EFFECT_PATH, SOUND_EFFECT_INTERFACE, (method))

// gesture 触控板手势
//#define GESTURE_SERVICE          "com.deepin.daemon.Gesture"
//#define GESTURE_PATH             "/com/deepin/daemon/Gesture"
//#define GESTURE_INTERFACE        "com.deepin.daemon.Gesture"
#define GESTURE_SERVICE          "org.deepin.dde.Gesture1"
#define GESTURE_PATH             "/org/deepin/dde/Gesture1"
#define GESTURE_INTERFACE        "org.deepin.dde.Gesture1"
#define GESTURE_SIGNAL           "Event"

// deepin wm 窗管
#define WM_SERVICE                       "com.deepin.wm"
#define WM_PATH                         "/com/deepin/wm"
#define WM_INTERFACE                    "com.deepin.wm"
#define WM_WORKSPACESWITCHED            "WorkspaceSwitched"

#define dbusPlaySound(sound) QDBusConnection::sessionBus().call(SOUND_EFFECT_METHOD("PlaySound")<<(sound))
#define dbusIsSoundEnabled(sound) QDBusConnection::sessionBus().call(SOUND_EFFECT_METHOD("IsSoundEnabled")<<(sound))
#define dbusEnableSound(sound, enable) QDBusConnection::sessionBus().call(SOUND_EFFECT_METHOD("EnableSound")<<(sound)<<(enable))

/*******************************************************************************
 1. @类名:    DBusManager
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:
*******************************************************************************/

class DBusManager : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", TERMINALSERVER)
public:
    DBusManager();
    ~DBusManager();
    /**
     * @brief 初始化terminal相关dbus
     * @author ut000610 戴正文
     * @return
     */
    bool initDBus();

    // kwin dbus
    /**
     * @brief 获取当前桌面index
     * @author ut000610 戴正文
     * @return
     */
    static int callKDECurrentDesktop();
    /**
     * @brief 将桌面跳转到index所指桌面
     * @author ut000610 戴正文
     * @param index
     */
    static void callKDESetCurrentDesktop(int index);

    // deepin terminal
    // 创建窗口
    /**
     * @brief 调用主进程的创建或显示窗口入口
     * @author ut000610 戴正文
     * @param args
     */
    static void callTerminalEntry(QStringList args);


    /** add by ut001121 zhangmeng 20200720 用于 sp3 键盘交互功能*/
    /**
     * @brief 调用系统音效
     * @author ut001121 张猛
     * @param sound 音效名称
                    错误提示:dialog-error
                    通   知:message
                    唤   醒:suspend-resume
                    音量调节:audio-volume-change
                    设备接入:device-added
                    设备拔出:device-removed
     */
    static void callSystemSound(const QString &sound = "dialog-error");

    /**
     * @brief 监听触控板事件
     * @author ut000610 戴正文
     */
    void listenTouchPadSignal();
    /**
     * @brief 监听桌面(工作区切换)
     * @author ut000610 戴正文
     */
    void listenDesktopSwitched();

public slots:
    /**
     * @brief dbus上开放的槽函数，用于调用service的Entry接口
     * @author ut000610 戴正文
     * @param args
     */
    void entry(QStringList args);

signals:
    // 该信号由Service在main入口中使用
    void entryArgs(QStringList args);
};

#endif // DBUSMANAGER_H
