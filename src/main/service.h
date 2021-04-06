/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:  wangpeili<wangpeili@uniontech.com>
 *
 * Maintainer:wangpeili<wangpeili@uniontech.com>
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
#ifndef SERVICE_H
#define SERVICE_H

#include "termproperties.h"
#include "windowsmanager.h"
#include "shortcutmanager.h"
#include "settings.h"
#include "customthemesettingdialog.h"

#include <DSettingsDialog>
#include <DDialog>
#include <QObject>
#include <QSharedMemory>
#include <QMap>

#include <com_deepin_wmswitcher.h>

using WMSwitcher = com::deepin::WMSwitcher;
#define WMSwitcherService "com.deepin.WMSwitcher"
#define WMSwitcherPath "/com/deepin/WMSwitcher"


DWIDGET_USE_NAMESPACE
struct ShareMemoryInfo {
    int enableCreateTerminal = 0;
    int TerminalsCount = 0;
    qint64 appStartTime = 0; //sub app 启动的时间
};
/*******************************************************************************
 1. @类名:    Service
 2. @作者:    ut000439 王培利
 3. @日期:    2020-05-19
 4. @说明:    全局的后台服务类．
　　　　　　　　管理所有底层数据的加载
　　　　　　　　管理所有特殊的通知信号
　　　　　　　　管理所有程序入口
*******************************************************************************/
class Service : public QObject
{
    Q_OBJECT
public:
    static Service *instance();
    ~Service();
    /**
     * @brief 初始化
     * @author ut000439 wangpeili
     */
    void init();
    /**
     * @brief 初始化设置框，在窗口现实后初始化，使第一次出现设置框不至于卡顿
     * @author ut000610 戴正文
     */
    void initSetting();

    /**
     * @brief 唯一显示设置框
     * @author ut000610 戴正文
     * @param pOwner
     */
    void showSettingDialog(MainWindow *pOwner);

    /**
     * @brief 显示自定义主题设置框
     * @author ut000125 sunchengxi
     * @param pOwner
     */
    void showCustomThemeSettingDialog(MainWindow *pOwner);
    /**
     * @brief 显示设置快捷键冲突弹窗
     * @author ut000610 戴正文
     * @param txt
     */
    void showShortcutConflictMsgbox(QString txt);
    // 设置框是否显示
    bool isSettingDialogVisible()
    {
        return m_settingDialog ? m_settingDialog->isVisible() : false;
    }
    //获取设置框的所有者
    MainWindow *getSettingOwner()
    {
        return m_settingOwner;
    }
    //重置设置框的所有者
    void resetSettingOwner()
    {
        m_settingOwner = nullptr;
    }

    /**
     * @brief i从term数量的角度判断是否允许继续创建
     * @author ut000610 戴正文
     * @return
     */
    bool isCountEnable();

    /**
     * @brief 获取是否是对话框显示
     * @author ut000439 wangpeili
     * @return
     */
    bool getIsDialogShow() const;
    /**
     * @brief 设置是否对话框显示
     * @author ut000439 wangpeili
     * @param parent
     * @param isDialogShow
     */
    void setIsDialogShow(QWidget *parent, bool isDialogShow);

    // 主进程是否允许创建新的窗口，如果允许，把子进程的创建时间设置到共享内存传递过去。
    /**
     * @brief 子进程获取是否可以创建窗口许可，获取到权限立即将标志位置为false
              增加子进程启动的时间，如果可以创建，把该时间写入共享内存，当创建mainwindow的时候，取走这个数据
     * @author ut000439 王培利
     * @param time
     * @return
     */
    bool getEnable(qint64 time) ;
    /**
     * @brief 设置子进程启动时间到共享内存
     * @author ut000439 王培利
     */
    void setSubAppStartTime(qint64);
    /**
     * @brief 获取子进程在共享的启动时间
     * @author ut000439 王培利
     * @return
     */
    qint64 getSubAppStartTime();

    /**
     * @brief ShareMemoryCount这个为当前总的终端数，但是雷神创建的第一个不包括在内
     * @author ut000439 王培利
     * @param count 总的终端数
     */
    void updateShareMemoryCount(int count);
    /*******************************************************************************
     1. @函数:    getShareMemoryCount
     2. @作者:    ut000439 wangpeili
     3. @日期:    2020-08-11
     4. @说明:    获取共享内存计数
    *******************************************************************************/
    /**
     * @brief 获取共享内存计数
     * @author ut000439 wangpeili
     * @return
     */
    int  getShareMemoryCount();
    /**
     * @brief 设置共享内存信息
     * @author ut000439 王培利
     * @param enable 1=true(主进程), 0=false(主进程首次或子进程获得许可以后)
     * @return
     */
    bool setMemoryEnable(bool enable);
    /**
     * @brief 获取共享内存标志， 1=true,0=false
     * @author ut000439 王培利
     * @return
     */
    bool getMemoryEnable();
    /**
     * @brief 释放共享内存连接
     * @author ut000439 王培利
     */
    void releaseShareMemory();

    /**
     * @brief 判断当前是否开启窗口特效  开启-true 关闭-false
     * @author ut000438 王亮
     * @return
     */
    bool isWindowEffectEnabled();
    /**
     * @brief 获取主程序初始进入的时间
     * @author ut000439 wangpeili
     * @return
     */
    qint64 getEntryTime();

    /**
     * @brief 从/etc/shells获取shell列表
     * @author ut000610 戴正文
     * @return
     */
    QMap<QString, QString> getShells();
    /**
     * @brief 获取shells的map,不读取/etc/shells
     * @author ut000610 戴正文
     * @return
     */
    QMap<QString, QString> shellsMap();

signals:
    void refreshCommandPanel(QString oldCmdName, QString newCmdName);
    // 切换编码列表的编码
    void checkEncode(QString encode);
    // 当前终端切换
    void currentTermChange(QWidget *term);
    // 窗口特效是否开启
    void onWindowEffectEnabled(bool isEnabled);
    // 触控板事件 name 事件名称 direction 手势方向 fingers 手指个数
    void touchPadEventSignal(QString name, QString direction, int fingers);
    void hostnameChanged();

public slots:
    /**
     * @brief 创建窗口的入口,Service进入接口
     * @author ut000439 wangpeili
     * @param arguments
     */
    void Entry(QStringList arguments);
    /**
     * @brief 对桌面工作区切换事件的处理
     * @author ut000610 戴正文
     * @param curDesktop 当前工作区
     * @param nextDesktop 下一个工作区
     */
    void onDesktopWorkspaceSwitched(int curDesktop, int nextDesktop);

    void slotShowSettingsDialog();

    /**
     * @brief DSettingsDialog对话框关闭后的处理
     * @author ut000438 王亮
     * @param result
     */
    void handleSettingsDialogFinished(int result);

    /**
     * @brief 处理窗口特效打开/关闭时，相关设置项目的显示/隐藏
     * @author ut000438 王亮
     * @param wmName
     */
    void handleWMChanged(const QString &wmName);

    /**
     * @brief 自定义主题对话框关闭后的处理
     * @author ut000438 王亮
     * @param result
     */
    void handleCustomThemeSettingDialogFinished(int result);

    void handleSettingShortcutConflictDialogFinished();

private:
    explicit Service(QObject *parent = nullptr);
    /**
     * @brief 显示/隐藏设置的透明度和背景模糊选项-- 仅UOS服务器版本使用
     * @author ut000438 王亮
     * @param isShow
     */
    void showHideOpacityAndBlurOptions(bool isShow);
    /**
     * @brief 监听窗口特效开关对应DBus信号，并实时显示/隐藏设置的透明度和背景模糊选项
     * @author ut000438 王亮
     */
    void listenWindowEffectSwitcher();

    static Service *g_pService;
    // 设置框 全局唯一显示
    DSettingsDialog *m_settingDialog = nullptr;

    //自定义主题设置对话框 全局唯一
    CustomThemeSettingDialog *m_customThemeSettingDialog = nullptr;

    WMSwitcher *m_wmSwitcher = nullptr;
    //设置框的所有者
    MainWindow *m_settingOwner = nullptr;
    // 设置框，快捷键冲突弹窗
    DDialog *m_settingShortcutConflictDialog = nullptr;
    // 雷神用来判断是否有弹窗显示
    bool m_isDialogShow = false;
    // 是否允许创建新的窗口
    bool m_enable = false;
    QSharedMemory *m_enableShareMemory = nullptr;
    // 这个指针实际上与上面指针指向同一地址，不需要二次释放
    ShareMemoryInfo *m_pShareMemoryInfo = nullptr;
    // 初始化和运行无障碍辅助工具的线程
//    AtspiDesktop *m_atspiThread = nullptr;

    // shells
    QMap<QString, QString> m_shellsMap;

    // 记录进入的时间，只有创建窗口时，才会来取用这个时间
    qint64 m_EntryTime = 0;
};

#endif // SERVICE_H
