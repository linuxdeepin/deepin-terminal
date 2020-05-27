#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "termproperties.h"
#include "titlebar.h"
#include "termwidgetpage.h"
#include "remotemanagementplugn.h"
#include "utils.h"

#include <DMainWindow>
#include <DWidgetUtil>
#include <DToolButton>
#include <DIconButton>
#include <DPushButton>

#include <QStackedWidget>
#include <QVBoxLayout>
#include <QSettings>
#include <QMouseEvent>

#include <functional>
#include <QShortcut>


DWIDGET_USE_NAMESPACE

class TabBar;
class TermWidgetPage;
class TermProperties;
class ShortcutManager;
class MainWindowPluginInterface;
class CustomCommandPlugin;

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

    void focusPage(const QString &identifier);
    void focusCurrentPage();
    TermWidgetPage *currentPage();
    void setCurrentPage(TermWidgetPage *page);
    TermWidgetPage *getPageByIdentifier(const QString &identifier);

    void forAllTabPage(const std::function<void(TermWidgetPage *)> &func);

    ShortcutManager *getShortcutManager();

    void executeDownloadFile();

    void pressCtrlAt();
    void pressCtrlU();
    void pressCtrlC();
    void sleep(unsigned int msec);

    // 由mainwindow统一指令当前显示哪个插件
    void showPlugin(const QString &name);
    // 快速隐藏插件
    void hidePlugin();
    // 新建工作区
    void createNewWorkspace();
    // 由mainwindow统一获取当前选择的文本。
    QString selectedText(bool preserveLineBreaks = true);

    static QList<MainWindow *> getWindowList();

    bool hasRunningProcesses();
    void switchFullscreen(bool forceFullscreen = false);
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
    static constexpr const char *CONFIG_QUAKE_WINDOW_HEIGHT = "quake_window_Height";
    /******** Add by nt001000 renfeixiang 2020-05-25:增加 定义 End***************/
    int getDesktopIndex() const;

    /******** Add by nt001000 renfeixiang 2020-05-20:增加雷神窗口根据字体和字体大小设置最小高度函数 Begin***************/
    //雷神窗口根据字体和字体大小设置最小高度
    void setQuakeWindowMinHeight();
    /******** Add by nt001000 renfeixiang 2020-05-20:增加雷神窗口根据字体和字体大小设置最小高度函数 End***************/

    // 处理雷神窗口丢失焦点自动隐藏功能
    void onAppFocusChangeForQuake();

signals:
    void newWindowRequest(const QString &directory);
    // !这两个信号被封装了，请不要单独调用！
    void showPluginChanged(const QString &name);
    void quakeHidePlugin();
    void mainwindowClosed(MainWindow *);
    // 编码方式，仅当前mainwindow有效．
    void changeEncodeSig(const QString &name);

public slots:
    // void onSettingValueChanged(const int &keyIndex, const QVariant &value);
    void onWindowSettingChanged(const QString &keyName);
    void onShortcutSettingChanged(const QString &keyName);
    void remoteUploadFile();
    void remoteDownloadFile();
    // 处理雷神窗口自动隐藏功能以及window+D一起显示的问题
    void onApplicationStateChanged(Qt::ApplicationState state);

protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

    //--added by qinyaning(nyq) to solve After exiting the pop-up interface,
    /*press Windows+D on the keyboard, the notification bar will
    *open the terminal interface to only display the exit
    *pop-up, click exit pop-up terminal interface to display abnormal
    */
    void changeEvent(QEvent *event) override;
    //------------------------------------------------------------
protected slots:
    void onTermTitleChanged(QString title);
    void onTabTitleChanged(QString title);
    void onCreateNewWindow(QString workingDir);

private:
    void initUI();
    void initWindow();
    // 雷神窗口
    void setQuakeWindow();
    void setNormalWindow();
    void setDefaultLocation();
    QString getConfigWindowState();
    QSize halfScreenSize();
    // 加载插件
    void initPlugins();
    void initShortcuts();
    void initConnections();
    void initTitleBar();
    void initOptionButton();
    void initOptionMenu();

    void setNewTermPage(TermWidgetPage *termPage, bool activePage = true);

    QString getWinInfoConfigPath();
    void initWindowPosition(MainWindow *mainwindow);
    void handleTitleBarMenuFocusPolicy();
    int executeCMD(const char *cmd);
    //void addQuakeTerminalShortcut();

    bool isTabVisited(int tabSessionId);
    bool isTabChangeColor(int tabSessionId);
    void updateTabStatus();
    void saveWindowSize();
    /******** Modify by n014361 wangpeili 2020-03-09: 非DTK控件手动匹配系统主题的修改 **********/
    void applyTheme();
    /********************* Modify by n014361 wangpeili End ************************/
    int getAllterminalCount();
    /**************** Modify by n013252 wangliang 2020-01-20: 终端退出保护 ****************/
    bool closeConfirm();
    /**************** Modify by n013252 wangliang End ****************/

    MainWindowPluginInterface *getPluginByName(const QString &name);

    /******** Modify by n014361 wangpeili 2020-01-06:增加显示快捷键功能***********×****/
    // 显示快捷键功能
    void displayShortcuts();
    // 创建Json组信息
    void createJsonGroup(const QString &keyCategory, QJsonArray &jsonGroups);
    /********************* Modify by n014361 wangpeili End ************************/
    /******** Modify by n014361 wangpeili 2020-02-20: 创建快捷键管理 ****************/
    QShortcut *createNewShotcut(const QString &key, bool AutoRepeat = true);
    /********************* Modify by n014361 wangpeili End ************************/
    //--added by qinyaning(nyq) to slove Unable to download file from server, time: 2020.4.13 18:21--//
    void pressEnterKey(const QString &text);
    //---------------------------------------

    QMenu *m_menu = nullptr;
    DToolButton *m_exitFullScreen = nullptr;
    TabBar *m_tabbar = nullptr;
    QWidget *m_centralWidget = nullptr;
    QVBoxLayout *m_centralLayout = nullptr;
    QStackedWidget *m_termStackWidget = nullptr;
    //QString m_titlebarStyleSheet = nullptr;
    ShortcutManager *m_shortcutManager = nullptr;
    QList<MainWindowPluginInterface *> m_plugins;
    TermProperties m_properties;
    TitleBar *m_titleBar = nullptr;
    bool m_isQuakeWindow = false;
    QMap<int, bool> m_tabVisitMap;
    QMap<int, bool> m_tabChangeColorMap;

    /******** Modify by n014361 wangpeili 2020-02-20: 内置快捷键集中管理 ****************/
    QMap<QString, QShortcut *> m_BuiltInShortcut;
    /********************* Modify by n014361 wangpeili End ************************/
    QString downloadFilePath = "";
    RemoteManagementPlugn *remoteManagPlugin = nullptr;
    CustomCommandPlugin *customCommandPlugin = nullptr;
    QString m_strWindowId;
    const char *cmd = "ps -elf|grep deepin-terminal$ |wc -l";

    /******** Modify by m000714 daizhengwen 2020-03-29: 终端窗口配置 ****************/
    QSettings *m_winInfoConfig = nullptr;
    /********************* Modify by m000714 daizhengwen End ************************/
    QString m_CurrentShowPlugin = PLUGIN_TYPE_NONE;

    static QList<MainWindow *> m_windowList;
private:
    //--added by qinyaning(nyq) to solve After exiting the pop-up interface,
    /* press Windows+D on the keyboard, the notification bar will
    *  open the terminal interface to only display the exit
    *  pop-up, click exit pop-up terminal interface to display abnormal, time: 2020.4.16 13:32
    */
    bool _isClickedExitDlg {false};

    // 窗口最小宽度
    const int m_MinWidth = 450;
    // 窗口最小高度
    const int m_MinHeight = 250;
    // 是否需要保存位置开关，雷神窗口不关心这个参数
    bool m_IfUseLastSize = false;
    // 雷神终端所在桌面
    int m_desktopIndex;
    bool m_hasConfirmedClose = false;
    // 应用程序状态
    static Qt::ApplicationState g_appState;
};

#endif  // MAINWINDOW_H
