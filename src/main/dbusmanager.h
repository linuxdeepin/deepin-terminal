// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUSMANAGER_H
#define DBUSMANAGER_H
#include "service.h"

#include <DSysInfo>

#include <QObject>
#include <QList>
#include <QVariant>

// deepin-terminal
#define TERMINALSERVER "com.deepin.terminal.reborn"
#define TERMINALINTERFACE "/window"

// kwin dbus
#define KWINDBUSSERVICE "org.kde.KWin"
#define KWINDBUSPATH "/KWin"

// deepin Appearance
inline const char *getAppearanceService() { return Dtk::Core::DSysInfo::majorVersion().toInt() >= 23 ? "org.deepin.dde.Appearance1" : "com.deepin.daemon.Appearance"; }
inline const char *getAppearancePath() { return Dtk::Core::DSysInfo::majorVersion().toInt() >= 23 ? "/org/deepin/dde/Appearance1" : "/com/deepin/daemon/Appearance"; }
#define APPEARANCESERVICE getAppearanceService()
#define APPEARANCEPATH getAppearancePath()

// sound effect 音效服务
inline const char *getSoundEffectService() { return Dtk::Core::DSysInfo::majorVersion().toInt() >= 23 ? "org.deepin.dde.SoundEffect1" : "com.deepin.daemon.SoundEffect"; }
inline const char *getSoundEffectPath() { return Dtk::Core::DSysInfo::majorVersion().toInt() >= 23 ? "/org/deepin/dde/SoundEffect1" : "/com/deepin/daemon/SoundEffect"; }
inline const char *getSoundEffectInterface() { return Dtk::Core::DSysInfo::majorVersion().toInt() >= 23 ? "org.deepin.dde.SoundEffect1" : "com.deepin.daemon.SoundEffect"; }
#define SOUND_EFFECT_METHOD(method) QDBusMessage::createMethodCall(getSoundEffectService(), getSoundEffectPath(), getSoundEffectInterface(), (method))

// gesture 触控板手势
inline const char *getGestureService() { return Dtk::Core::DSysInfo::majorVersion().toInt() >= 23 ? "org.deepin.dde.Gesture1" : "com.deepin.daemon.Gesture"; }
inline const char *getGesturePath() { return Dtk::Core::DSysInfo::majorVersion().toInt() >= 23 ? "/org/deepin/dde/Gesture1" : "/com/deepin/daemon/Gesture"; }
inline const char *getGestureInterface() { return Dtk::Core::DSysInfo::majorVersion().toInt() >= 23 ? "org.deepin.dde.Gesture1" : "com.deepin.daemon.Gesture"; }
#define GESTURE_SERVICE          getGestureService()
#define GESTURE_PATH             getGesturePath()
#define GESTURE_INTERFACE        getGestureInterface()
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

    // Appearance
    /**
     * @brief callAppearanceFont 获取有效的字体列表
     * @param fontList 字体id-list
     * @param fontType 字体类型如：monospacefont、otherfailfont
     * @return
     */
    static FontDataList callAppearanceFont(QStringList fontList, QString fontType);
    static FontDataList callAppearanceFont(QString fontType);

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

    int consoleFontSize() const;   // 字体大小
    void setConsoleFontSize(const int size);

    QString consoleFontFamily() const;   // 字体名称
    void setConsoleFontFamily(const QString family);

    qreal consoleOpacity() const;   // 控制台透明度值
    void setConsoleOpacity(const int value);

    int consoleCursorShape() const;   // 光标样式值
    void setConsoleCursorShape(const int shape);

    bool consoleCursorBlink() const;   // 光标是否闪烁
    void setConsoleCursorBlink(const bool blink);

    QString consoleColorScheme() const;   // 颜色主题
    void setConsoleColorScheme(const QString scheme);

    QString consoleShell() const; // shell 类型
    void setConsoleShell(const QString shellName);

signals:
    // 该信号由Service在main入口中使用
    void entryArgs(QStringList args);
};

#endif // DBUSMANAGER_H
