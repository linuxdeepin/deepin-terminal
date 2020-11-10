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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "termproperties.h"
#include "titlebar.h"
#include "termwidgetpage.h"
#include "remotemanagementplugn.h"
#include "utils.h"
#include "define.h"

// dtk
#include <DMainWindow>
#include <DWidgetUtil>
#include <DToolButton>
#include <DIconButton>
#include <DPushButton>

// qt
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QSettings>
#include <QMouseEvent>
#include <QTimer>
#include <QMenu>
#include <QDebug>
#include <QShortcut>

#include <functional>


DWIDGET_USE_NAMESPACE

class TabBar;
class TermWidgetPage;
class TermProperties;
class ShortcutManager;
class MainWindowPluginInterface;
class CustomCommandPlugin;


/*******************************************************************************
 1. @类名:    SwitchThemeMenu
 2. @作者:    ut000125 sunchengxi
 3. @日期:    2020-10-28
 4. @说明:    主题菜单的快捷键项在鼠标离开悬浮时，触发主题还原
*******************************************************************************/
class SwitchThemeMenu : public QMenu
{
    Q_OBJECT
public:
    SwitchThemeMenu(const QString &title, QWidget *parent = nullptr);
    //捕获鼠标离开主题项事件
    void leaveEvent(QEvent *) override;
    //主题菜单栏隐藏时触发
    void hideEvent(QHideEvent *) override;
    //捕获鼠标进入主题项事件
    void enterEvent(QEvent *event) override;
    //处理键盘主题项左键按下离开事件
    void keyPressEvent(QKeyEvent *event) override;

signals:
    //主题项在鼠标停靠离开时触发的信号
    void mainWindowCheckThemeItemSignal();

    void menuHideSetThemeSignal();
public:
    //鼠标悬殊主题记录，防止频繁刷新，鼠标再次进入主题列表负责刷新预览
    QString hoveredThemeStr = "";
};


/*******************************************************************************
 1. @类名:    MainWindow
 2. @作者:    ut000439 王培利
 3. @日期:    2020-07-31
 4. @说明:    主窗口
             基类，不可直接生成对象，派生普测窗口和雷神窗口
             分发插件显隐信号
             控制TAB-PAGE增减
             捆绑全局快捷键
*******************************************************************************/
class MainWindow : public DMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(TermProperties properties, QWidget *parent = nullptr);
    ~MainWindow() override;

    void addTab(TermProperties properties, bool activeTab = false);

    /******** Modify by n014361 wangpeili 2020-01-07:  关闭其它标签页功能 ************/
    // 点击，右键，快捷键以及被调用．
    void closeTab(const QString &identifier, bool hasConfirmed = false);
    // Tab右键或者快捷键
    void closeOtherTab(const QString &identifier, bool hasConfirmed = false);
    // 整体关闭事件
    void closeAllTab();
    /********************* Modify by n014361 wangpeili End ************************/
    void showExitConfirmDialog(Utils::CloseType type, int count = 1, QWidget *parent = nullptr);

    /************************ Mod by sunchengxi 2020-04-30:分屏修改标题异常问题 Begin************************/
    QString getCurrTabTitle();
    /************************ Mod by sunchengxi 2020-04-30:分屏修改标题异常问题 End  ************************/
    // 焦点是否在列表上
    bool isFocusOnList();

    // 上传指定的文件
    void remoteUploadFile(QString strFileNames);

    void focusPage(const QString &identifier);
    void focusCurrentPage();
    TermWidgetPage *currentPage();
    void setCurrentPage(TermWidgetPage *page);
    TermWidgetPage *getPageByIdentifier(const QString &identifier);

    void forAllTabPage(const std::function<void(TermWidgetPage *)> &func);

    // 基类终端是否闲置响应函数(闲置即没有程序运行)
    void onTermIsIdle(QString tabIdentifier, bool bIdle);

    void executeDownloadFile();

    void pressCtrlAt();
    void pressCtrlU();
    void pressCtrlC();
    void sleep(int msec);

    // 由mainwindow统一指令当前显示哪个插件
    void showPlugin(const QString &name);
    // 快速隐藏插件
    void hidePlugin();
    // 新建标签页
    void createNewTab();
    // 由mainwindow统一获取当前选择的文本。
    QString selectedText(bool preserveLineBreaks = true);

    // 新建窗口时间(ms)
    qint64 createNewMainWindowTime();

    bool hasRunningProcesses();
    bool isQuakeMode();

    static constexpr const char *PLUGIN_TYPE_SEARCHBAR = "Search Bar";
    static constexpr const char *PLUGIN_TYPE_THEME = "Theme";
    static constexpr const char *PLUGIN_TYPE_CUSTOMCOMMAND = "Custom Command";
    static constexpr const char *PLUGIN_TYPE_REMOTEMANAGEMENT = "Remote Management";
    static constexpr const char *PLUGIN_TYPE_ENCODING = "Encoding";
    static constexpr const char *PLUGIN_TYPE_NONE = "None";
    /******** Add by nt001000 renfeixiang 2020-05-25:增加 定义 Begin***************/
    static constexpr const char *CONFIG_WINDOW_WIDTH = "window_width";
    static constexpr const char *CONFIG_WINDOW_HEIGHT = "window_height";
    //Modify by ut001000 renfeixiang 2020-06-18:修改quake_window_Height为quake_window_height bug#33779
    static constexpr const char *CONFIG_QUAKE_WINDOW_HEIGHT = "quake_window_height";
    /******** Add by nt001000 renfeixiang 2020-05-25:增加 定义 End***************/
    static constexpr const char *QKEYSEQUENCE_FOCUSOUT_TIMINAL = "Meta+Tab";
    static constexpr const char *QKEYSEQUENCE_PASTE_BUILTIN = "Shift+Ins";
    static constexpr const char *QKEYSEQUENCE_COPY_BUILTIN = "Ctrl+Ins";

    /******** Add by ut001000 renfeixiang 2020-08-07:将m_MinWidth和m_MinHeight设置成全局变量***************/
    // 窗口最小宽度
    static const int m_MinWidth;
    // 窗口最小高度
    static const int m_MinHeight;

    //主题菜单
    static constexpr const char *THEME                          = "Theme";
    //浅色主题
    static constexpr const char *THEME_SYSTEN_LIGHT             = "Light Theme";
    //深色主题
    static constexpr const char *THEME_SYSTEN_DARK              = "Dark Theme";
    //跟随系统
    static constexpr const char *THEME_SYSTEN                   = "System Theme";

    //浅色主题
    static constexpr const char *THEME_LIGHT                    = "Light";
    //深色主题
    static constexpr const char *THEME_DARK                     = "Dark";
    //无内置主题
    static constexpr const char *THEME_NO                       = "";
    //内置主题1
    static constexpr const char *THEME_ONE                      = "Theme1";
    static constexpr const char *THEME_ONE_NAME                 = "Elementary";
    //内置主题2
    static constexpr const char *THEME_TWO                      = "Theme2";
    static constexpr const char *THEME_TWO_NAME                 = "Empathy";
    //内置主题3
    static constexpr const char *THEME_THREE                    = "Theme3";
    static constexpr const char *THEME_THREE_NAME               = "Tomorrow night blue";
    //内置主题4
    static constexpr const char *THEME_FOUR                     = "Theme4";
    static constexpr const char *THEME_FOUR_NAME                = "Bim";
    //内置主题5
    static constexpr const char *THEME_FIVE                     = "Theme5";
    static constexpr const char *THEME_FIVE_NAME                = "Freya";
    //内置主题6
    static constexpr const char *THEME_SIX                      = "Theme6";
    static constexpr const char *THEME_SIX_NAME                 = "Hybrid";
    //内置主题7
    static constexpr const char *THEME_SEVEN                    = "Theme7";
    static constexpr const char *THEME_SEVEN_NAME               = "Ocean dark";
    //内置主题8
    static constexpr const char *THEME_EIGHT                    = "Theme8";
    static constexpr const char *THEME_EIGHT_NAME               = "Deepin";
    //内置主题9
    static constexpr const char *THEME_NINE                     = "Theme9";
    static constexpr const char *THEME_NINE_NAME                = "Ura";
    //内置主题10
    static constexpr const char *THEME_TEN                      = "Theme10";
    static constexpr const char *THEME_TEN_NAME                 = "One light";

    // /etc/hostname 文件路径
    static constexpr const char *HOSTNAME_PATH                 = "/etc/hostname";
    int getDesktopIndex() const;

signals:
    void newWindowRequest(const QString &directory);
    // !这两个信号被封装了，请不要单独调用！
    void showPluginChanged(const QString &name, bool bSetFocus = false);
    void quakeHidePlugin();
    void mainwindowClosed(MainWindow *);
    // 编码方式，仅当前mainwindow有效．
    void changeEncodeSig(const QString &name);

public slots:
    void onWindowSettingChanged(const QString &keyName);
    void onShortcutSettingChanged(const QString &keyName);
    void remoteUploadFile();
    void remoteDownloadFile();
    // 处理雷神窗口自动隐藏功能以及window+D一起显示的问题
    void onApplicationStateChanged(Qt::ApplicationState state);
    void addCustomCommandSlot(QAction *newAction);
    void removeCustomCommandSlot(QAction *newAction);

    /******** Add by ut001000 renfeixiang 2020-06-03:增加 Begin***************/
    //处理关闭类型的参数
    void OnHandleCloseType(int result, Utils::CloseType type);
    /******** Add by ut001000 renfeixiang 2020-06-03:增加 End***************/

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

    //------------------------------------------------------------

    //选中当前的内置主题项
    void checkExtendThemeItem(const QString &expandThemeStr, QAction *&action);
    //选中当前的主题项
    void checkThemeItem();
    //增加主题菜单
    void addThemeMenuItems();
    //选择主题项
    void switchThemeAction(QAction *);
    //选择内置主题项
    void switchThemeAction(QAction *&action, const QString &themeNameStr);


protected slots:
    void onTermTitleChanged(QString title);
    void onTabTitleChanged(QString title);
    void onCreateNewWindow(QString workingDir);

    //鼠标选择时主题切换触发的槽函数
    void themeActionTriggeredSlot(QAction *);
    //鼠标悬浮主题项时切换触发的槽函数
    void themeActionHoveredSlot(QAction *);
    //设置选中的主题项的槽函数
    void setThemeCheckItemSlot();
    //主题菜单隐藏时设置主题槽函数
    void menuHideSetThemeSlot();

protected:
    void initUI();
    void initWindow();
    void setDefaultLocation();
    void singleFlagMove();
    QString getConfigWindowState();
    QSize halfScreenSize();
    // 加载插件
    void initPlugins();
    void initShortcuts();
    void initConnections();
    // 初始化标签
    void initTabBar();
    void initOptionButton();
    void initOptionMenu();
    //初始化文件检测
    void initFileWatcher();
    void setNewTermPage(TermWidgetPage *termPage, bool activePage = true);

    QString getWinInfoConfigPath();
    void initWindowPosition(MainWindow *mainwindow);
    void handleTitleBarMenuFocusPolicy();
    int executeCMD(const char *cmd);

    bool isTabVisited(const QString &tabIdentifier);
    bool isTabChangeColor(const QString &tabIdentifier);
    void updateTabStatus();
    // 非DTK控件手动匹配系统主题的修改
    void applyTheme();

    // 终端退出保护
    bool closeConfirm();

    MainWindowPluginInterface *getPluginByName(const QString &name);

    // 显示快捷键功能
    void displayShortcuts();
    // 创建Json组信息
    void createJsonGroup(const QString &keyCategory, QJsonArray &jsonGroups);
    // 创建快捷键管理
    QShortcut *createNewShotcut(const QString &key, bool AutoRepeat = true);
    //--added by qinyaning(nyq) 2020.4.13 18:21 解决无法从服务器下载文件的问题//
    void pressEnterKey(const QString &text);

    // mainwindow创建结束记录
    void createWindowComplete();
    // 首个终端创建成功结束
    void firstTerminalComplete();


protected:
    // 初始化标题栏
    virtual void initTitleBar() = 0;
    // 初始化窗口属性
    virtual void initWindowAttribute() = 0;

public:
    // 保存窗口尺寸
    virtual void saveWindowSize() = 0;
    // 切换全屏
    virtual void switchFullscreen(bool forceFullscreen = false) = 0;
    // 显示快捷键功能
    virtual QPoint calculateShortcutsPreviewPoint() = 0;
    // 处理雷神窗口丢失焦点自动隐藏功能
    virtual void onAppFocusChangeForQuake() = 0;
    // 根据字体和字体大小设置最小高度
    virtual void setWindowMinHeightForFont() = 0;
    /******** Add by ut001000 renfeixiang 2020-08-07:基类中定义更新最小高度函数的纯虚函数***************/
    virtual void updateMinHeight() = 0;

protected:
    QMenu *m_menu = nullptr;
    TabBar *m_tabbar = nullptr;
    QWidget *m_centralWidget = nullptr;
    QVBoxLayout *m_centralLayout = nullptr;
    QStackedWidget *m_termStackWidget = nullptr;
    QList<MainWindowPluginInterface *> m_plugins;
    TermProperties m_properties;
    TitleBar *m_titleBar = nullptr;
    bool m_isQuakeWindow = false;
    QMap<QString, bool> m_tabVisitMap;
    QMap<QString, bool> m_tabChangeColorMap;

    /******** Modify by n014361 wangpeili 2020-02-20: 内置快捷键集中管理 ****************/
    QMap<QString, QShortcut *> m_builtInShortcut;
    /********************* Modify by n014361 wangpeili End ************************/
    QString downloadFilePath = "";
    RemoteManagementPlugin *remoteManagPlugin = nullptr;
    CustomCommandPlugin *customCommandPlugin = nullptr;
    QString m_strWindowId;
    const char *cmd = "ps -elf|grep deepin-terminal$ |wc -l";

    /******** Modify by m000714 daizhengwen 2020-03-29: 终端窗口配置 ****************/
    QSettings *m_winInfoConfig = nullptr;
    /********************* Modify by m000714 daizhengwen End ************************/
    QString m_CurrentShowPlugin = PLUGIN_TYPE_NONE;

protected:
    // 是否需要保存位置开关，雷神窗口不关心这个参数
    bool m_IfUseLastSize = false;
    // 雷神终端所在桌面
    int m_desktopIndex;
    bool m_hasConfirmedClose = false;

    // 对应的程序启动时间，主进程或子进程
    qint64 m_ReferedAppStartTime = 0;
    // MainWindow开始创建的时间
    qint64 m_CreateWindowTime = 0;
    // MainWindow创建结束的时间
    qint64 m_WindowCompleteTime = 0;
    // 第一个Terminal创建完成时间
    qint64 m_FirstTerminalCompleteTime = 0;

    // ID
    int m_MainWindowID = 0;
    // 创建第一个终端完成时，需要记录
    bool hasCreateFirstTermialComplete = false;

    //主题菜单
    SwitchThemeMenu *switchThemeMenu        = nullptr;
    //浅色主题快捷键
    QAction         *lightThemeAction       = nullptr;
    //深色主题快捷键
    QAction         *darkThemeAction        = nullptr;
    //跟随系统主题快捷键
    QAction         *autoThemeAction        = nullptr;

    //内置主题1快捷键
    QAction         *themeOneAction         = nullptr;
    //内置主题2快捷键
    QAction         *themeTwoAction         = nullptr;
    //内置主题3快捷键
    QAction         *themeThreeAction       = nullptr;
    //内置主题4快捷键
    QAction         *themeFourAction        = nullptr;
    //内置主题5快捷键
    QAction         *themeFiveAction        = nullptr;
    //内置主题6快捷键
    QAction         *themeSixAction         = nullptr;
    //内置主题7快捷键
    QAction         *themeSevenAction       = nullptr;
    //内置主题8快捷键
    QAction         *themeEightAction       = nullptr;
    //内置主题9快捷键
    QAction         *themeNineAction        = nullptr;
    //内置主题10快捷键
    QAction         *themeTenAction         = nullptr;

    //主题快捷键组
    QActionGroup    *group                  = nullptr;

    //当前勾选的菜单主题快捷键
    QAction         *currCheckThemeAction   = nullptr;

};

/*******************************************************************************
 1. @类名:    NormalWindow
 2. @作者:    ut001121 zhangmeng
 3. @日期:    2020-08-11
 4. @说明:    普通终端窗口
*******************************************************************************/
class NormalWindow : public MainWindow
{
    Q_OBJECT

public:
    explicit NormalWindow(TermProperties properties, QWidget *parent = nullptr);
    ~NormalWindow() override;
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
};

/*******************************************************************************
 1. @类名:    QuakeWindow
 2. @作者:    ut001121 zhangmeng
 3. @日期:    2020-08-11
 4. @说明:    雷神终端窗口
*******************************************************************************/
class QuakeWindow : public MainWindow
{
    Q_OBJECT

    // 雷神resize是否拖拽的状态
    enum Quake_Resize_State {
        // 不进行resize
        Quake_NoResize = 0,
        // 正在resize
        Quake_Resize = 1,
        // 准备resize
        Quake_Prepare_Resize = 2
    };

public:
    explicit QuakeWindow(TermProperties properties, QWidget *parent = nullptr);
    ~QuakeWindow() override;

    /******** Add by ut001000 renfeixiang 2020-08-07:用于雷神窗口增加和减少横向分屏时，对雷神窗口的自小高进行修改，bug#41436***************/
    virtual void updateMinHeight() override;

    // 是否在记录的当前桌面显示
    bool isShowOnCurrentDesktop();
    // 隐藏雷神终端
    void hideQuakeWindow();
    // 切换窗口拉伸属性
    inline void switchEnableResize();

public slots:
    // 处理resize消息
    void onResizeWindow();

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
    virtual void setWindowMinHeightForFont() override;

protected:
    void changeEvent(QEvent *event) override;
    void showEvent(QShowEvent *event) override;
    bool event(QEvent *event) override;
    // 事件过滤器 => 处理雷神resize
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    // 雷神窗口resize状态 默认noresize
    Quake_Resize_State m_resizeState = Quake_NoResize;
    // 雷神窗口的高度
    int m_quakeWindowHeight;
    // 雷神resize的定时器
    QTimer *m_resizeTimer = nullptr;
    // 记录雷神是否当前桌面显示
    QMap<int, bool> m_desktopMap;
};

#endif  // MAINWINDOW_H
