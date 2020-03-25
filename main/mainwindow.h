#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "termproperties.h"
#include "titlebar.h"
#include "termwidgetpage.h"
#include "remotemanagementplugn.h"

#include <DMainWindow>

#include <QStackedWidget>
#include <QVBoxLayout>

#include <functional>
#include <DPushButton>
#include <QShortcut>

#define PLUGIN_TYPE_THEME "Theme"
#define PLUGIN_TYPE_CUSTOMCOMMAND "Custom Command"
#define PLUGIN_TYPE_REMOTEMANAGEMENT "Remote Management"
#define PLUGIN_TYPE_ENCODING "Encoding"

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

    bool isTabVisited(int tabSessionId);
    void addTab(TermProperties properties, bool activeTab = false);
    void closeTab(const QString &identifier);
    void updateTabStatus();

    /******** Modify by n014361 wangpeili 2020-01-07:  关闭其它标签页功能 ************/
    void closeOtherTab();
    /********************* Modify by n014361 wangpeili End ************************/

    void focusTab(const QString &identifier);
    TermWidgetPage *currentTab();

    void forAllTabPage(const std::function<void(TermWidgetPage *)> &func);
    void setTitleBarBackgroundColor(QString color);
    void setQuakeWindow(bool isQuakeWindow);
    ShortcutManager *getShortcutManager();

    /********** Modify by n013252 wangliang 2020-01-14: 是否主动激活主窗口 **********/
    bool isQuakeWindowActivated();
    void setQuakeWindowActivated(bool isQuakeWindowActivated);
    /**************** Modify by n013252 wangliang End ****************/

    void executeDownloadFile();
    QString showFileDailog(bool isDir);
    void pressCtrlAt();
    void sleep(unsigned int msec);
signals:
    void newWindowRequest(const QString &directory);

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

protected slots:
    void onTermTitleChanged(QString title);
    void onTabTitleChanged(QString title);
    void onCreateNewWindow(QString workingDir);

private:
    void initUI();
    void initPlugins();
    void initWindow();
    void initShortcuts();
    void initConnections();
    void initTitleBar();
    void setNewTermPage(TermWidgetPage *termPage, bool activePage = true);
    void showSettingDialog();
    QString getWinInfoConfigPath();
    void initWindowPosition(MainWindow *mainwindow);
    void handleTitleBarMenuFocusPolicy();
    int executeCMD(const char *cmd);
    void addQuakeTerminalShortcut();

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
    QMenu *m_menu = nullptr;
    DPushButton *m_exitFullScreen = nullptr;
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

    /******** Modify by n014361 wangpeili 2020-02-20: 内置快捷键集中管理 ****************/
    QMap<QString, QShortcut *> m_BuiltInShortcut;
    /********************* Modify by n014361 wangpeili End ************************/
    QString downloadFilePath = "";
    bool enterSzCommand = false;
    RemoteManagementPlugn *remoteManagPlugin;
    CustomCommandPlugin *customCommandPlugin;
    QString m_strWindowId;
    const char *cmd = "ps -elf|grep deepin-terminal$ |wc -l";
};

#endif  // MAINWINDOW_H
