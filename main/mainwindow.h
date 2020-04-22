#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "termproperties.h"
#include "titlebar.h"
#include "termwidgetpage.h"
#include "remotemanagementplugn.h"

#include <DMainWindow>
#include <DWidgetUtil>
#include <DToolButton>
#include <DIconButton>
#include <DPushButton>

#include <QStackedWidget>
#include <QVBoxLayout>
#include <QSettings>

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
    void closeTab(const QString &identifier, bool runCheck = true);

    /******** Modify by n014361 wangpeili 2020-01-07:  关闭其它标签页功能 ************/
    void closeOtherTab(const QString &identifier);
    /********************* Modify by n014361 wangpeili End ************************/

    void focusPage(const QString &identifier);
    void focusCurrentPage();
    TermWidgetPage *currentPage();
    TermWidgetPage *getPageByIdentifier(const QString &identifier);

    void forAllTabPage(const std::function<void(TermWidgetPage *)> &func);
    void setTitleBarBackgroundColor(QString color);

    ShortcutManager *getShortcutManager();

    /********** Modify by n013252 wangliang 2020-01-14: 是否主动激活主窗口 **********/
    bool isQuakeWindowActivated();
    void setQuakeWindowActivated(bool isQuakeWindowActivated);
    /**************** Modify by n013252 wangliang End ****************/

    void executeDownloadFile();
    QString showFileDailog(bool isDir);
    void pressCtrlAt();
    void pressCtrlU();
    void sleep(unsigned int msec);

    // 由mainwindow统一指令当前显示哪个插件
    void showPlugin(const QString &name);
    // 快速隐藏插件
    void hidePlugin();
    // 由mainwindow统一获取当前选择的文本。
    QString selectedText(bool preserveLineBreaks = true);

    static QList<MainWindow *> getWindowList();

    bool hasRunningProcesses();

    static constexpr const char *PLUGIN_TYPE_SEARCHBAR = "Search Bar";
    static constexpr const char *PLUGIN_TYPE_THEME = "Theme";
    static constexpr const char *PLUGIN_TYPE_CUSTOMCOMMAND = "Custom Command";
    static constexpr const char *PLUGIN_TYPE_REMOTEMANAGEMENT = "Remote Management";
    static constexpr const char *PLUGIN_TYPE_ENCODING = "Encoding";
    static constexpr const char *PLUGIN_TYPE_NONE = "None";
signals:
    void newWindowRequest(const QString &directory);
    // !这两个信号被封装了，请不要单独调用！
    void showPluginChanged(const QString &name);
    void quakeHidePlugin();

public slots:
    // void onSettingValueChanged(const int &keyIndex, const QVariant &value);
    void onWindowSettingChanged(const QString &keyName);
    void onShortcutSettingChanged(const QString &keyName);
    void remoteUploadFile();
    void remoteDownloadFile();

protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

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
    void resizeAndMove();
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
    void showSettingDialog();
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

    /**************** Modify by n013252 wangliang 2020-01-20: 终端退出保护 ****************/
    bool closeProtect();
    /**************** Modify by n013252 wangliang End ****************/

    /******** Modify by n014361 wangpeili 2020-02-18:              ****************/
    void switchFullscreen(bool forceFullscreen = false);
    /********************* Modify by n014361 wangpeili End ************************/

    MainWindowPluginInterface *getPluginByName(const QString &name);

    /******** Modify by n014361 wangpeili 2020-01-06:增加显示快捷键功能***********×****/
    // 显示快捷键功能
    void displayShortcuts();
    // 创建Json组信息
    void createJsonGroup(const QString &keyCategory, QJsonArray &jsonGroups);
    /********************* Modify by n014361 wangpeili End ************************/
    /******** Modify by n014361 wangpeili 2020-02-20: 创建快捷键管理 ****************/
    QShortcut *createNewShotcut(const QString &key);
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
    bool m_isQuakeWindowActivated = false;
    QMap<int, bool> m_tabVisitMap;
    QMap<int, bool> m_tabChangeColorMap;

    /******** Modify by n014361 wangpeili 2020-02-20: 内置快捷键集中管理 ****************/
    QMap<QString, QShortcut *> m_BuiltInShortcut;
    /********************* Modify by n014361 wangpeili End ************************/
    QString downloadFilePath = "";
    bool enterSzCommand = false;
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
    // 是否需要保存位置开关
    bool m_IfUseLastSize = false;
};

#endif  // MAINWINDOW_H
