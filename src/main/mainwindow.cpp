#include "mainwindow.h"

#include "mainwindowplugininterface.h"
#include "settings.h"
#include "shortcutmanager.h"
#include "tabbar.h"
#include "termproperties.h"
#include "termwidgetpage.h"
#include "termwidget.h"
#include "titlebar.h"
#include "operationconfirmdlg.h"
#include "encodepanelplugin.h"
#include "customcommandplugin.h"
#include "serverconfigmanager.h"
#include "utils.h"
#include "dbusmanager.h"
#include "windowsmanager.h"
#include "service.h"

#include <DSettings>
#include <DSettingsGroup>
#include <DSettingsOption>
#include <DSettingsWidgetFactory>
#include <DThemeManager>
#include <DTitlebar>
#include <DFileDialog>
#include <DLog>

#include <QApplication>
#include <QDesktopWidget>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMouseEvent>
#include <QProcess>
#include <QSettings>
#include <QStandardPaths>
#include <QtDBus>

#include <QVBoxLayout>
#include <QMap>
#include <DImageButton>

#include <fstream>
using std::ifstream;
using std::ofstream;

DWIDGET_USE_NAMESPACE

QList<MainWindow *> MainWindow::m_windowList;
Qt::ApplicationState MainWindow::g_appState = Qt::ApplicationInactive;

MainWindow::MainWindow(TermProperties properties, QWidget *parent)
    : DMainWindow(parent),
      m_menu(new QMenu),
      m_tabbar(nullptr),
      m_centralWidget(new QWidget(this)),
      m_centralLayout(new QVBoxLayout(m_centralWidget)),
      m_termStackWidget(new QStackedWidget),
      m_properties(properties),
      m_isQuakeWindow(properties[QuakeMode].toBool()),
      m_winInfoConfig(new QSettings(getWinInfoConfigPath(), QSettings::IniFormat))
{
    m_windowList.append(this);
    initUI();
    initConnections();
    initShortcuts();
}

void MainWindow::initUI()
{
    initWindow();
    // Plugin may need centralWidget() to work so make sure initPlugin() is after setCentralWidget()
    // Other place (eg. create titlebar menu) will call plugin method so we should create plugins before init other
    // parts.
    initPlugins();
    initTitleBar();

    // 雷神和普通窗口设置不同
    m_isQuakeWindow ? setQuakeWindow() : setNormalWindow();
    addTab(m_properties);
    m_desktopIndex = DBusManager::callKDECurrentDesktop();

    /******** Add by nt001000 renfeixiang 2020-05-20:增加setQuakeWindowMinHeight函数，设置雷神最小高度 Begin***************/
    setQuakeWindowMinHeight();
    /******** Add by nt001000 renfeixiang 2020-05-20:增加setQuakeWindowMinHeight函数，设置雷神最小高度 End***************/
    //下面代码待处理
    ShortcutManager::instance()->setMainWindow(this);
    m_shortcutManager = ShortcutManager::instance();

    qDebug() << m_termStackWidget->size();
    qApp->installEventFilter(this);
}
void MainWindow::initWindow()
{
    setAttribute(Qt::WA_TranslucentBackground);
    setMinimumSize(m_MinWidth, m_MinHeight);
    setEnableBlurWindow(Settings::instance()->backgroundBlur());
    setWindowIcon(QIcon::fromTheme("deepin-terminal"));

    // Init layout
    m_centralLayout->setMargin(0);
    m_centralLayout->setSpacing(0);
    m_centralLayout->addWidget(m_termStackWidget);
    setCentralWidget(m_centralWidget);
}
/*******************************************************************************
 1. @函数:    initTitleBar
 2. @作者:    n014361 王培利
 3. @日期:    2020-04-22
 4. @说明:    TitleBar初始化，分雷神窗口和普通窗口
*******************************************************************************/
void MainWindow::initTitleBar()
{
    m_tabbar = new TabBar(this);
    m_tabbar->setFocusPolicy(Qt::NoFocus);

    connect(m_tabbar, &DTabBar::tabBarClicked, this, [ = ](int index) {
        TermWidgetPage *tabPage = qobject_cast<TermWidgetPage *>(m_termStackWidget->widget(index));
        TermWidget *term = tabPage->currentTerminal();
        bool bIdle = !(term->hasRunningProcess());

        int currSessionId = term->getSessionId();
        if (bIdle && isTabChangeColor(currSessionId)) {
            m_tabVisitMap.insert(currSessionId, true);
            m_tabChangeColorMap.insert(currSessionId, false);
            m_tabbar->removeNeedChangeTextColor(index);
        }
    });

    // 点击TAB上页触发
    connect(m_tabbar,
            &DTabBar::currentChanged,
            this,
    [this](int index) {
        focusPage(m_tabbar->identifier(index));
    },
    Qt::QueuedConnection);
    // 点击TAB上的＂＋＂触发
    connect(m_tabbar, &DTabBar::tabAddRequested, this, [this]() {
        createNewWorkspace();
    }, Qt::QueuedConnection);

    // 点击TAB上的＂X＂触发
    connect(m_tabbar,
            &DTabBar::tabCloseRequested,
            this,
    [this](int index) {
        closeTab(m_tabbar->identifier(index));
    },
    Qt::QueuedConnection);

    // TAB菜单发来的关闭请求
    connect(m_tabbar, &TabBar::menuCloseTab, this, [ = ](QString Identifier) {
        closeTab(Identifier);
        return;
    });

    // TAB菜单发来的关闭其它窗口请求,需要逐一关闭
    connect(m_tabbar, &TabBar::menuCloseOtherTab, this, [ = ](QString Identifier) {
        closeOtherTab(Identifier);
        return;
    });



    // titleba在普通模式和雷神模型不一样的功能
    m_titleBar = new TitleBar(this, m_isQuakeWindow);
    m_titleBar->setTabBar(m_tabbar);
    // 雷神的特殊处理
    if (m_isQuakeWindow) {
        titlebar()->setFixedHeight(0);
        m_centralLayout->addWidget(m_titleBar);
    } else {
        titlebar()->setCustomWidget(m_titleBar);
        titlebar()->setAutoHideOnFullscreen(true);
        titlebar()->setTitle("");

        //设置titlebar焦点策略为不抢占焦点策略，防止点击titlebar后终端失去输入焦点
        titlebar()->setFocusPolicy(Qt::NoFocus);
        initOptionButton();
        initOptionMenu();

        //fix bug 17566 正常窗口下，新建和关闭窗口菜单栏会高亮
        //handleTitleBarMenuFocusPolicy();
    }

    // titlebar所有控件不可获取焦点
    Utils::clearChildrenFocus(titlebar());
}
/*******************************************************************************
 1. @函数:    initOptionButton
 2. @作者:    n014361 王培利
 3. @日期:    2020-04-22
 4. @说明:    普通模式下，option button需要在全屏时切换控件
*******************************************************************************/
void MainWindow::initOptionButton()
{
    // 全屏退出按钮
    // DTK的全屏按钮不能满足UI要求，隐去DTK最右侧的全屏
    QWidget *dtkbutton = titlebar()->findChild<QWidget *>("DTitlebarDWindowQuitFullscreenButton");
    if (dtkbutton) {
        dtkbutton->hide();
    }

    m_exitFullScreen = new DToolButton(this);
    m_exitFullScreen->setCheckable(false);
    m_exitFullScreen->setIcon(QIcon::fromTheme("dt_exit_fullscreen"));
    m_exitFullScreen->setIconSize(QSize(36, 36));
    m_exitFullScreen->setFixedSize(QSize(36, 36));
    titlebar()->addWidget(m_exitFullScreen, Qt::AlignRight | Qt::AlignHCenter);
    m_exitFullScreen->setVisible(false);
    connect(m_exitFullScreen, &DPushButton::clicked, this, [this]() {
        switchFullscreen();
    });

    // option button
    DIconButton *optionBtn = titlebar()->findChild<DIconButton *>("DTitlebarDWindowOptionButton");
    if (optionBtn != nullptr) {
        optionBtn->setFocusPolicy(Qt::NoFocus);
        // mainwindow的设置按钮触发
        connect(titlebar()->findChild<DIconButton *>("DTitlebarDWindowOptionButton"), &DIconButton::pressed, this, [this]() {
            showPlugin(PLUGIN_TYPE_NONE);
        });
    }
}
/*******************************************************************************
 1. @函数:    initOptionMenu
 2. @作者:    n014361 王培利
 3. @日期:    2020-04-22
 4. @说明:    option menu初始化
*******************************************************************************/
void MainWindow::initOptionMenu()
{
    titlebar()->setMenu(m_menu);
    /******** Modify by m000714 daizhengwen 2020-04-03: 新建窗口****************/
    QAction *newWindowAction(new QAction(tr("New &window"), this));
    connect(newWindowAction, &QAction::triggered, this, [this]() {
        qDebug() << "menu click new window";

        TermWidgetPage *tabPage = currentPage();
        TermWidget *term = tabPage->currentTerminal();
        QString currWorkingDir = term->workingDirectory();
        emit newWindowRequest(currWorkingDir);
    });
    m_menu->addAction(newWindowAction);
    /********************* Modify by m000714 daizhengwen End ************************/
    for (MainWindowPluginInterface *plugin : m_plugins) {
        QAction *pluginMenu = plugin->titlebarMenu(this);
        // 取消Encoding插件的菜单展示
        if (plugin->getPluginName() == PLUGIN_TYPE_ENCODING) {
            continue;
        }
        if (pluginMenu) {
            m_menu->addAction(pluginMenu);
        }
    }

    QAction *settingAction(new QAction(tr("&Settings"), this));
    m_menu->addAction(settingAction);
    m_menu->addSeparator();
    connect(settingAction, &QAction::triggered, Service::instance(), [ = ] {
        Service::instance()->showSettingDialog(this);
    });
}

void MainWindow::initPlugins()
{
    // Todo: real plugin loader and plugin support.
    // ThemePanelPlugin *testPlugin = new ThemePanelPlugin(this);
    // testPlugin->initPlugin(this);
    EncodePanelPlugin *encodePlugin = new EncodePanelPlugin(this);
    encodePlugin->initPlugin(this);

    customCommandPlugin = new CustomCommandPlugin(this);
    customCommandPlugin->initPlugin(this);

    remoteManagPlugin = new RemoteManagementPlugn(this);
    remoteManagPlugin->initPlugin(this);

    m_plugins.append(encodePlugin);
    m_plugins.append(customCommandPlugin);
    m_plugins.append(remoteManagPlugin);
}

MainWindow::~MainWindow()
{
    m_windowList.removeOne(this);
}

//void MainWindow::addQuakeTerminalShortcut()
//{
//    QDBusInterface shortcutInterface("com.deepin.daemon.Keybinding",
//                                     "/com/deepin/daemon/Keybinding",
//                                     "com.deepin.daemon.Keybinding",
//                                     QDBusConnection::sessionBus());
//    if (!shortcutInterface.isValid()) {
//        qWarning() << "com.deepin.daemon.Keybinding error ," << shortcutInterface.lastError().name();
//        return;
//    }

//    QVariant shortcutName(QString("雷神终端"));
//    QVariant shortcutAction(QString("deepin-terminal -q"));
//    QVariant shortcutKeySequence(QString("<Alt>F2"));
//    QDBusReply<void> reply = shortcutInterface.asyncCall("AddCustomShortcut", shortcutName, shortcutAction, shortcutKeySequence);
//    if (reply.isValid()) {
//        qDebug() << "AddCustomShortcut success..";
//    } else {
//        qDebug() << "AddCustomShortcut failed..";
//    }
//}
/*******************************************************************************
 1. @函数:    setQuakeWindow
 2. @作者:    n014361 王培利
 3. @日期:    2020-04-22
 4. @说明:    雷神窗口的特殊设置
*******************************************************************************/
void MainWindow::setQuakeWindow()
{
    /************************ Add by m000743 sunchengxi 2020-04-27:雷神窗口任务栏移动后位置异常问题 Begin************************/
    setWindowRadius(0);
    //QRect deskRect = QApplication::desktop()->availableGeometry();//获取可用桌面大小
    QDesktopWidget *desktopWidget = QApplication::desktop();
    QRect screenRect = desktopWidget->screenGeometry(); //获取设备屏幕大小
    Qt::WindowFlags windowFlags = this->windowFlags();
    setWindowFlags(windowFlags | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint /*| Qt::Dialog*/);

    //add a line by ut001121 zhangmeng 2020-04-27雷神窗口禁用移动(修复bug#22975)
    setEnableSystemMove(false);//    setAttribute(Qt::WA_Disabled, true);

    /******** Modify by m000714 daizhengwen 2020-03-26: 窗口高度超过２／３****************/
    setMinimumSize(screenRect.size().width(), 60);
    setMaximumHeight(screenRect.size().height() * 2 / 3);
    /********************* Modify by m000714 daizhengwen End ************************/
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setFixedWidth(QApplication::desktop()->availableGeometry().width());
    connect(desktopWidget, &QDesktopWidget::workAreaResized, this, [this]() {
        qDebug() << "workAreaResized" << QApplication::desktop()->availableGeometry();
        /******** Modify by nt001000 renfeixiang 2020-05-20:修改成只需要设置雷神窗口宽度,根据字体高度设置雷神最小高度 Begin***************/
        setMinimumWidth(QApplication::desktop()->availableGeometry().width());
        setQuakeWindowMinHeight();
        /******** Modify by nt001000 renfeixiang 2020-05-20:修改成只需要设置雷神窗口宽度,根据字体高度设置雷神最小高度 End***************/
        move(QApplication::desktop()->availableGeometry().x(), QApplication::desktop()->availableGeometry().y());
        qDebug() << "size" << size();
        setFixedWidth(QApplication::desktop()->availableGeometry().width());
        return ;
    });

    /******** Modify by nt001000 renfeixiang 2020-05-25: 文件wininfo-config.conf中参数,使用定义更换quake_window_Height Begin***************/
    int saveHeight = m_winInfoConfig->value(CONFIG_QUAKE_WINDOW_HEIGHT).toInt();
    /******** Modify by nt001000 renfeixiang 2020-05-25: 文件wininfo-config.conf中参数,使用定义更换quake_window_Height End***************/
    qDebug() << "quake_window_Height: " << saveHeight;
    qDebug() << "quake_window_Height: " << minimumSize();
    // 如果配置文件没有数据
    if (saveHeight == 0) {
        saveHeight = screenRect.size().height() / 3;
    }
    int saveWidth = screenRect.size().width();
    resize(QSize(saveWidth, saveHeight));
    move(0, 0);
    /************************ Add by m000743 sunchengxi 2020-04-27:雷神窗口任务栏移动后位置异常问题 End  ************************/
}
/*******************************************************************************
 1. @函数:    setNormalWindow
 2. @作者:    n014361 王培利
 3. @日期:    2020-04-22
 4. @说明:    标准模式的窗口设置
*******************************************************************************/
void MainWindow::setNormalWindow()
{
    // init window state.
    QString windowState = getConfigWindowState();
    if (windowState == "window_maximum") {
        setDefaultLocation();
        showMaximized();
    } else if (windowState == "fullscreen") {
        setDefaultLocation();
        switchFullscreen(true);
    } else if (windowState == "split_screen") {
        setWindowRadius(0);
        resize(halfScreenSize());
    } else {
        m_IfUseLastSize = true;
        /******** Modify by nt001000 renfeixiang 2020-05-25: 文件wininfo-config.conf中参数,使用定义更换window_width，window_height Begin***************/
        int saveWidth = m_winInfoConfig->value(CONFIG_WINDOW_WIDTH).toInt();
        int saveHeight = m_winInfoConfig->value(CONFIG_WINDOW_HEIGHT).toInt();
        qDebug() << "window_width: " << saveWidth;
        qDebug() << "window_height: " << saveHeight;
        /******** Modify by nt001000 renfeixiang 2020-05-25:文件wininfo-config.conf中参数,使用定义更换window_width，window_height End***************/
        // 如果配置文件没有数据
        if (saveWidth == 0 || saveHeight == 0) {
            saveWidth = 1000;
            saveHeight = 600;
        }
        resize(QSize(saveWidth, saveHeight));

        if (m_properties[SingleFlag].toBool()) {
            Dtk::Widget::moveToCenter(this);
            qDebug() << "SingleFlag move" ;
        }
    }
}
/*******************************************************************************
 1. @函数:    setDefaultLocation
 2. @作者:    n014361 王培利
 3. @日期:    2020-04-22
 4. @说明:    设置默认位置，最大化，全屏还原使用
*******************************************************************************/
void MainWindow::setDefaultLocation()
{
    resize(QSize(1000, 600));
    if (m_properties[SingleFlag].toBool()) {
        Dtk::Widget::moveToCenter(this);
        qDebug() << "SingleFlag move" ;
    }
}

bool MainWindow::isTabVisited(int tabSessionId)
{
    return m_tabVisitMap.value(tabSessionId);
}

bool MainWindow::isTabChangeColor(int tabSessionId)
{
    return m_tabChangeColorMap.value(tabSessionId);
}

void MainWindow::addTab(TermProperties properties, bool activeTab)
{
    /***add by ut001121 zhangmeng 修复BUG#24452 点击“+”按钮新建工作区，自定义命令/编码/远程管理插件未消失***/
    showPlugin(PLUGIN_TYPE_NONE);

    if (WindowsManager::instance()->widgetCount() >= TermWidget::MaxTermwidgetCount) {
        qDebug() << "addTab failed, can't create number more than 200";
        return;
    }
    TermWidgetPage *termPage = new TermWidgetPage(properties, this);
    WindowsManager::instance()->setCurrentPage(termPage);
    setNewTermPage(termPage, activeTab);

    // pageID存在 tab中，所以page增删改操作都要由tab发起。
    int index = m_tabbar->addTab(termPage->identifier(), m_tabbar->tabText(m_tabbar->currentIndex()));
    qDebug() << "addTab index" << index;
    if (activeTab) {
        m_tabbar->setCurrentIndex(index);
        m_tabbar->removeNeedChangeTextColor(index);
    }

    TermWidget *term = termPage->currentTerminal();
    m_tabbar->saveSessionIdWithTabIndex(term->getSessionId(), index);
    m_tabbar->saveSessionIdWithTabId(term->getSessionId(), termPage->identifier());
    connect(termPage, &TermWidgetPage::termTitleChanged, this, &MainWindow::onTermTitleChanged);
    connect(termPage, &TermWidgetPage::tabTitleChanged, this, &MainWindow::onTabTitleChanged);
    connect(termPage, &TermWidgetPage::lastTermClosed, this, [this](const QString & identifier) {
        closeTab(identifier);
    });
    /******** Modify by m000714 daizhengwen 2020-03-31: 避免多次菜单弹出****************/
    // 菜单弹出在时间过滤器获取，不需要从terminal事件中获取
//    connect(termPage, &TermWidgetPage::termGetFocus, this, [ = ]() {
//        showPlugin(PLUGIN_TYPE_NONE);
//    });
    /********************* Modify by m000714 daizhengwen End ************************/
    connect(this, &MainWindow::showPluginChanged, termPage, [ = ](const QString name) {
        termPage->showSearchBar(PLUGIN_TYPE_SEARCHBAR == name && (this->currentPage() == termPage));
    });

    connect(termPage->currentTerminal(), &TermWidget::termIsIdle, this, [ = ](int currSessionId, bool bIdle) {

        int tabIndex = m_tabbar->queryIndexBySessionId(currSessionId);

        //如果标签被点过，移除标签颜色
        if (isTabVisited(currSessionId) && bIdle) {
            m_tabVisitMap.insert(currSessionId, false);
            m_tabChangeColorMap.insert(currSessionId, false);
            m_tabbar->removeNeedChangeTextColor(tabIndex);
            return;
        }

        if (bIdle) {
            //空闲状态如果标签被标记变色，则改变标签颜色
            if (m_tabbar->isNeedChangeTextColor(tabIndex)) {
                m_tabChangeColorMap.insert(currSessionId, true);
                m_tabbar->setChangeTextColor(tabIndex);
            }
        } else {

            //如果当前标签是活动标签，移除变色请求
            int activeTabIndex = m_tabbar->currentIndex();
            if (activeTabIndex == tabIndex) {
                m_tabVisitMap.insert(currSessionId, false);
                m_tabChangeColorMap.insert(currSessionId, false);
                m_tabbar->removeNeedChangeTextColor(tabIndex);
                return;
            }

            //标记变色，发起请求，稍后等空闲状态变色
            m_tabChangeColorMap.insert(currSessionId, false);
            DGuiApplicationHelper *appHelper = DGuiApplicationHelper::instance();
            DPalette pa = appHelper->standardPalette(appHelper->themeType());
            m_tabbar->setNeedChangeTextColor(tabIndex, pa.color(DPalette::Highlight));
        }
    });
}

bool MainWindow::hasRunningProcesses()
{
    for (int i = 0, count = m_termStackWidget->count(); i < count; i++) {
        TermWidgetPage *tabPage = qobject_cast<TermWidgetPage *>(m_termStackWidget->widget(i));
        TermWidget *term = tabPage->currentTerminal();
        if (term->hasRunningProcess()) {
            qDebug() << "here are processes running in this terminal tab... " << tabPage->identifier() << endl;
            return true;
        } else {
            qDebug() << "no processes running in this terminal tab... " << tabPage->identifier() << endl;
        }
    }

    return false;
}
/*******************************************************************************
 1. @函数:    closeTab
 2. @作者:    n014361 王培利
 3. @日期:    2020-05-07
 4. @说明:    一个tab只提示一次, 检测后需要重入，hasCheck=true
             或者不需要提示，hasCheck=true即可
*******************************************************************************/
void MainWindow::closeTab(const QString &identifier, bool hasConfirmed)
{
    /***add by ut001121 zhangmeng 20200508 修复BUG#24457 点击标签栏“x”按钮，右键菜单关闭工作区，关闭其它工作区，自定义命令/编码/远程管理插件未消失***/
    showPlugin(PLUGIN_TYPE_NONE);

    TermWidgetPage *tabPage = getPageByIdentifier(identifier);
    if (tabPage == nullptr) {
        return;
    }
    // 关闭前必须要切换过去先
    if (m_tabbar->currentIndex() != m_tabbar->getIndexByIdentifier(identifier)) {
        m_tabbar->setCurrentIndex(m_tabbar->getIndexByIdentifier(identifier));
    }
    // 默认每个窗口关闭都提示一次．
    if (!hasConfirmed && tabPage->runningTerminalCount() != 0) {
        showExitConfirmDialog(Utils::CloseType_Tab, tabPage->runningTerminalCount(), this);
        return;
    }
    qDebug() << "Tab closed" << identifier;
    int currSessionId = tabPage->currentTerminal()->getSessionId();
    m_tabVisitMap.remove(currSessionId);
    m_tabChangeColorMap.remove(currSessionId);
    m_tabbar->removeTab(identifier);
    m_termStackWidget->removeWidget(tabPage);
    tabPage->deleteLater();

    if (m_tabbar->count() != 0) {
        updateTabStatus();
        focusCurrentPage();
        return;
    }
    qDebug() << "mainwindow close";
    close();
}
/*******************************************************************************
 1. @函数:    getAllterminalCount
 2. @作者:    n014361 王培利
 3. @日期:    2020-04-26
 4. @说明:    获取当前所有终端的最大值．
*******************************************************************************/
int MainWindow::getAllterminalCount()
{
    int termcount = 0;
    for (MainWindow *window : m_windowList) {
        for (int i = 0, count = window->m_termStackWidget->count(); i < count; i++) {
            TermWidgetPage *tabPage = qobject_cast<TermWidgetPage *>(window->m_termStackWidget->widget(i));
            termcount += tabPage->getTerminalCount();
        }
    }

    return termcount;
}

void MainWindow::updateTabStatus()
{
    for (int i = 0; i < m_tabbar->count(); i++) {
        TermWidgetPage *tabPage = qobject_cast<TermWidgetPage *>(m_termStackWidget->widget(i));
        TermWidget *term = tabPage->currentTerminal();
        bool bIdle = !(term->hasRunningProcess());
        int currSessionId = term->getSessionId();

        if (bIdle) {
            if (isTabVisited(currSessionId)) {
                m_tabVisitMap.insert(currSessionId, false);
                m_tabChangeColorMap.insert(currSessionId, false);
                m_tabbar->removeNeedChangeTextColor(i);
            } else if (isTabChangeColor(currSessionId)) {
                m_tabbar->setChangeTextColor(i);
            } else {
                m_tabbar->removeNeedChangeTextColor(i);
            }
        } else {
            m_tabbar->removeNeedChangeTextColor(i);
        }
    }
}

void MainWindow::saveWindowSize()
{
    // 雷神窗口保存
    if (m_isQuakeWindow) {
        // 记录最后一个正常窗口的大小
        /******** Modify by nt001000 renfeixiang 2020-05-25: 文件wininfo-config.conf中参数,使用定义更换quake_window_Height Begin***************/
        m_winInfoConfig->setValue(CONFIG_QUAKE_WINDOW_HEIGHT, height());
        /******** Modify by nt001000 renfeixiang 2020-05-25: 文件wininfo-config.conf中参数,使用定义更换quake_window_Height End***************/
        qDebug() << "save quake_window_Height:" << height();
        return;
    }

    // 过滤普通模式的特殊窗口
    if (!m_IfUseLastSize) {
        return;
    }
    // (真.假)半屏窗口大小时就不记录了
    if ((size() == halfScreenSize()) || (size() == (halfScreenSize() + QSize(0, 1)))) {
        return;
    }

    if (windowState() == Qt::WindowNoState) {
        /******** Modify by nt001000 renfeixiang 2020-05-25: 文件wininfo-config.conf中参数,使用定义更换window_width，window_height Begin***************/
        // 记录最后一个正常窗口的大小
        m_winInfoConfig->setValue(CONFIG_WINDOW_WIDTH, width());
        m_winInfoConfig->setValue(CONFIG_WINDOW_HEIGHT, height());
        qDebug() << "save windows size:" << width() << height();
        /******** Modify by nt001000 renfeixiang 2020-05-25: 文件wininfo-config.conf中参数,使用定义更换window_width，window_height End***************/
    }
}

QString MainWindow::getCurrTabTitle()
{
    return m_tabbar->tabText(m_tabbar->currentIndex());
}
/*******************************************************************************
 1. @函数:    closeOtherTab
 2. @作者:    n014361 王培利
 3. @日期:    2020-05-07
 4. @说明:    关闭其它窗口
*******************************************************************************/
void MainWindow::closeOtherTab(const QString &identifier, bool hasConfirmed)
{
    int runningCount = 0;
    QList<QString> closeTabIdList;
    for (int i = 0, count = m_termStackWidget->count(); i < count; i++) {
        TermWidgetPage *tabPage = qobject_cast<TermWidgetPage *>(m_termStackWidget->widget(i));
        if (tabPage && tabPage->identifier() != identifier) {
            closeTabIdList.append(tabPage->identifier());
            runningCount += tabPage->runningTerminalCount();
        }
    }

    if (!hasConfirmed && runningCount != 0) {
        // 全部关闭时，仅提示一次．
        showExitConfirmDialog(Utils::CloseType_OtherTab, runningCount, this);
        return;
    }

    // 关闭其它窗口，需要检测
    for (QString id : closeTabIdList) {
        closeTab(id, true);
        qDebug() << " close" << id;
    }

    //如果是不关闭当前页的，最后回到当前页来．
    m_tabbar->setCurrentIndex(m_tabbar->getIndexByIdentifier(identifier));

    return;
}
/*******************************************************************************
 1. @函数:     closeOtherTab
 2. @作者:     n014361 王培利
 3. @日期:     2020-01-10
 4. @说明:     关闭所有标签页功能
*******************************************************************************/
void MainWindow::closeAllTab()
{
    QList<QString> closeTabIdList;
    for (int i = 0, count = m_termStackWidget->count(); i < count; i++) {
        TermWidgetPage *tabPage = qobject_cast<TermWidgetPage *>(m_termStackWidget->widget(i));
        closeTabIdList.append(tabPage->identifier());
    }

    // 全部关闭时，不再检测了，
    for (QString id : closeTabIdList) {
        closeTab(id, true);
        qDebug() << " close" << id;
    }

    return;
}

void MainWindow::showExitConfirmDialog(Utils::CloseType type, int count, QWidget *parent)
{
    // count < 1 不提示
    if (count < 1) {
        return;
    }
    QString title;
    QString txt;
    Utils::CloseType temtype = type;
    // 关闭窗口的时候，如果只有一个tab,提示的内容要为终端．
    if (type == Utils::CloseType_Window && m_tabbar->count() == 1) {
        temtype = Utils::CloseType_Terminal;
    }
    Utils::getExitDialogText(temtype, title, txt, count);

    // 有弹窗显示
    Service::instance()->setIsDialogShow(this, true);

    DDialog *dlg = new DDialog(title, txt, parent);
    dlg->setIcon(QIcon::fromTheme("deepin-terminal"));
    dlg->addButton(QString(tr("Cancel")), false, DDialog::ButtonNormal);
    /******** Modify by nt001000 renfeixiang 2020-05-21:修改Exit成Close Begin***************/
    dlg->addButton(QString(tr("Close")), true, DDialog::ButtonWarning);
    /******** Modify by nt001000 renfeixiang 2020-05-21:修改Exit成Close End***************/
    dlg->setWindowModality(Qt::WindowModal);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->show();

    if (type == Utils::CloseType_Window) {
        connect(dlg, &DDialog::finished, this, [this](int result) {
            qDebug() << result;
            //setEnabled(true);
            // 弹窗隐藏或消失
            Service::instance()->setIsDialogShow(this, false);
            if (result == 1) {
                //接口二次重入
                m_hasConfirmedClose = true;
                close();
            }
        });
    }
    if (type == Utils::CloseType_Tab) {
        connect(dlg, &DDialog::finished, this, [this](int result) {
            qDebug() << result;
            //setEnabled(true);
            // 弹窗隐藏或消失
            Service::instance()->setIsDialogShow(this, false);
            if (result == 1) {
                TermWidgetPage *page = currentPage();
                if (page) {
                    //接口二次重入
                    closeTab(page->identifier(), true);
                }
            }
        });
    }

    if (type == Utils::CloseType_OtherTab) {
        connect(dlg, &DDialog::finished, this, [this](int result) {
            qDebug() << result;
            //setEnabled(true);
            // 弹窗隐藏或消失
            Service::instance()->setIsDialogShow(this, false);
            if (result == 1) {
                TermWidgetPage *page = currentPage();
                if (page) {
                    //接口二次重入
                    closeOtherTab(page->identifier(), true);
                }
            }
        });
    }

    //while (1) {
    //    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    //}
    //bool result = (dlg->exec() == DDialog::Accepted);
    //qDebug()<<"receive result = "<<result;
    return ;
}

void MainWindow::focusPage(const QString &identifier)
{
    TermWidgetPage *tabPage = getPageByIdentifier(identifier);
    if (tabPage) {
        m_termStackWidget->setCurrentWidget(tabPage);
        WindowsManager::instance()->setCurrentPage(tabPage);
        tabPage->focusCurrentTerm();
        return;
    }
    qDebug() << "focusTab nullptr identifier" << identifier;
}

void MainWindow::focusCurrentPage()
{
    focusPage(m_tabbar->identifier(m_tabbar->currentIndex()));
}

TermWidgetPage *MainWindow::currentPage()
{
    return WindowsManager::instance()->currentPage();
}

TermWidgetPage *MainWindow::getPageByIdentifier(const QString &identifier)
{
    for (int i = 0, count = m_termStackWidget->count(); i < count; i++) {
        TermWidgetPage *tabPage = qobject_cast<TermWidgetPage *>(m_termStackWidget->widget(i));
        if (tabPage && tabPage->identifier() == identifier) {
            return tabPage;
        }
    }
    qDebug() << "getPageByIdentifier nullptr identifier" << identifier;
    return nullptr;
}

void MainWindow::forAllTabPage(const std::function<void(TermWidgetPage *)> &func)
{
    for (int i = 0, count = m_termStackWidget->count(); i < count; i++) {
        TermWidgetPage *tabPage = qobject_cast<TermWidgetPage *>(m_termStackWidget->widget(i));
        if (tabPage) {
            func(tabPage);
        }
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    // 保存窗口位置
    saveWindowSize();
    // 通知隐藏插件
    hidePlugin();

    DMainWindow::resizeEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveWindowSize();

    // 注销和关机时不需要确认了
    if (qApp->isSavingSession()) {
        DMainWindow::closeEvent(event);
    }
    // 一页一页退出，当全部退出以后，mainwindow自然关闭．
    event->ignore();

    int runningCount = 0;
    QList<QString> closeTabIdList;
    for (int i = 0, count = m_termStackWidget->count(); i < count; i++) {
        TermWidgetPage *tabPage = qobject_cast<TermWidgetPage *>(m_termStackWidget->widget(i));
        closeTabIdList.append(tabPage->identifier());
        runningCount += tabPage->runningTerminalCount();
    }

    if (!m_hasConfirmedClose && runningCount != 0) {
        // 如果不能马上关闭，并且还在没有最小化．
        if (runningCount != 0  && isMinimized()) {
            qDebug() << "isMinimized........... " << endl;
            setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
        }

        // 全部关闭时，仅提示一次．
        showExitConfirmDialog(Utils::CloseType_Window, runningCount, this);
        return;
    }
    closeAllTab();


    if (m_tabbar->count() == 0) {
        DMainWindow::closeEvent(event);
        emit mainwindowClosed(this);
    }
}
/*******************************************************************************
 1. @函数:    closeConfirm
 2. @作者:    n014361 王培利
 3. @日期:    2020-04-29
 4. @说明:    mainwindow关闭确认
 　　　　　　　当前方案为一页一页确认
*******************************************************************************/
bool MainWindow::closeConfirm()
{
    int runningCount = 0;
    QList<QString> closeTabIdList;
    for (int i = 0, count = m_termStackWidget->count(); i < count; i++) {
        TermWidgetPage *tabPage = qobject_cast<TermWidgetPage *>(m_termStackWidget->widget(i));
        closeTabIdList.append(tabPage->identifier());
        runningCount += tabPage->runningTerminalCount();
    }

    // 如果不能马上关闭，并且还在没有最小化．
    if (runningCount != 0  && isMinimized()) {
        qDebug() << "isMinimized........... " << endl;
        setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    }
    if (runningCount != 0) {
        // 全部关闭时，仅提示一次．
        showExitConfirmDialog(Utils::CloseType_Window, runningCount, this);
        return false;
    }
    return true;
}

/*******************************************************************************
 1. @函数:    switchFullscreen
 2. @作者:    n014361 王培利
 3. @日期:    2020-02-18
 4. @说明:    全屏切换
*******************************************************************************/
void MainWindow::switchFullscreen(bool forceFullscreen)
{
    if (m_isQuakeWindow) {
        return;
    }
    if (forceFullscreen || !window()->windowState().testFlag(Qt::WindowFullScreen)) {
        window()->setWindowState(windowState() | Qt::WindowFullScreen);
    } else {
        window()->setWindowState(windowState() & ~Qt::WindowFullScreen);
    }
}

bool MainWindow::isQuakeMode()
{
    return  m_isQuakeWindow;
}

void MainWindow::onTermTitleChanged(QString title)
{
    TermWidgetPage *tabPage = qobject_cast<TermWidgetPage *>(sender());
    const bool customName = tabPage->property("TAB_CUSTOM_NAME_PROPERTY").toBool();
    if (!customName) {
        m_tabbar->setTabText(tabPage->identifier(), title);
    }
}

void MainWindow::onTabTitleChanged(QString title)
{
    TermWidgetPage *tabPage = qobject_cast<TermWidgetPage *>(sender());
    TermWidget *term = tabPage->currentTerminal();
    term->setProperty("currTabTitle", QVariant::fromValue(title));
    m_tabbar->setTabText(tabPage->identifier(), title);
}





QString MainWindow::getConfigWindowState()
{
    QString windowState =
        Settings::instance()->settings->option("advanced.window.use_on_starting")->value().toString();

    // 启动参数配置的状态值优先于 内部配置的状态值
    if (m_properties.contains(StartWindowState)) {
        QString state = m_properties[StartWindowState].toString();
        qDebug() << "use line state set:" << state;
        if (state == "maximize") {
            windowState = "window_maximum";
        } else if (state == "splitscreen") {
            windowState = "split_screen";
        } else if (state == "normal") {
            windowState = "window_normal";
        } else if (state == "fullscreen") {
            windowState = state;
        } else {
            qDebug() << "error line state set:" << state << "ignore it!";
        }
    }
    return  windowState;
}

/*******************************************************************************
 1. @函数:    halfScreenSize
 2. @作者:    n014361 王培利
 3. @日期:    2020-04-22
 4. @说明:    获取半屏大小（高度-1,如果不-1,最大后无法正常还原）
*******************************************************************************/
QSize MainWindow::halfScreenSize()
{
    QDesktopWidget w;
    int x = w.availableGeometry().width() / 2;
    int y = w.availableGeometry().height() - 1;
    QSize size(x, y);
    //qDebug() << "halfScreenSize:" << size;
    return size;
}

QString MainWindow::getWinInfoConfigPath()
{
    QDir winInfoPath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    if (!winInfoPath.exists()) {
        winInfoPath.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    }

    QString winInfoFilePath(winInfoPath.filePath("wininfo-config.conf"));
    return winInfoFilePath;
}

void MainWindow::initWindowPosition(MainWindow *mainwindow)
{
    int m_WindowNumber = executeCMD(cmd);
    if (m_WindowNumber == 1) {
        mainwindow->move((QApplication::desktop()->width() - width()) / 2, (QApplication::desktop()->height() - height()) / 2);
    }
}

void MainWindow::initShortcuts()
{
    m_shortcutManager->initShortcuts();
    /******** Modify by n014361 wangpeili 2020-01-10: 增加设置的各种快捷键修改关联***********×****/
    // new_workspace
    connect(createNewShotcut("shortcuts.workspace.new_workspace", false), &QShortcut::activated, this, [this]() {
        this->addTab(currentPage()->createCurrentTerminalProperties(), true);
    });

    // close_workspace
    connect(createNewShotcut("shortcuts.workspace.close_workspace"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentPage();
        if (page) {
            closeTab(page->identifier());
        }
    });

    // Close_other_workspaces
    connect(createNewShotcut("shortcuts.workspace.close_other_workspace"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentPage();
        if (page) {
            closeOtherTab(page->identifier());
        }
    });

    // previous_workspace
    connect(createNewShotcut("shortcuts.workspace.previous_workspace"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentPage();
        if (page) {
            int index = m_tabbar->currentIndex();
            index -= 1;
            if (index < 0) {
                index = m_tabbar->count() - 1;
            }
            m_tabbar->setCurrentIndex(index);
        }
    });

    // next_workspace
    connect(createNewShotcut("shortcuts.workspace.next_workspace"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentPage();
        if (page) {
            int index = m_tabbar->currentIndex();
            index += 1;
            if (index == m_tabbar->count()) {
                index = 0;
            }
            m_tabbar->setCurrentIndex(index);
        }
    });

    // horionzal_split
    connect(createNewShotcut("shortcuts.workspace.horionzal_split", false), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentPage();
        if (page) {
            qDebug() << "horizontal_split";
            page->split(Qt::Horizontal);
        }
    });

    // vertical_split
    connect(createNewShotcut("shortcuts.workspace.vertical_split", false), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentPage();
        if (page) {
            qDebug() << "vertical_split";
            page->split(Qt::Vertical);
        }
    });

    // select_upper_window
    connect(createNewShotcut("shortcuts.workspace.select_upper_window"), &QShortcut::activated, this, [this]() {
        qDebug() << "Alt+k";
        TermWidgetPage *page = currentPage();
        if (page) {
            page->focusNavigation(Qt::TopEdge);
        }
    });
    // select_lower_window
    connect(createNewShotcut("shortcuts.workspace.select_lower_window"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentPage();
        if (page) {
            page->focusNavigation(Qt::BottomEdge);
        }
    });
    // select_left_window
    connect(createNewShotcut("shortcuts.workspace.select_left_window"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentPage();
        if (page) {
            page->focusNavigation(Qt::LeftEdge);
        }
    });
    // select_right_window
    connect(createNewShotcut("shortcuts.workspace.select_right_window"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentPage();
        if (page) {
            page->focusNavigation(Qt::RightEdge);
            // QMouseEvent e(QEvent::MouseButtonPress, ) QApplication::sendEvent(focusWidget(), &keyPress);
        }
    });

    // close_window
    connect(createNewShotcut("shortcuts.workspace.close_window"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentPage();
        if (page) {
            qDebug() << "CloseWindow";
            page->closeSplit(page->currentTerminal());
        }
    });

    // close_other_windows
    connect(createNewShotcut("shortcuts.workspace.close_other_windows"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentPage();
        if (page) {
            page->closeOtherTerminal();
        }
    });

    // copy
    connect(createNewShotcut("shortcuts.terminal.copy"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentPage();
        if (page) {
            page->copyClipboard();
        }
    });

    // paste
    connect(createNewShotcut("shortcuts.terminal.paste"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentPage();
        if (page) {
            page->pasteClipboard();
        }
    });

    // search
    connect(createNewShotcut("shortcuts.terminal.find"), &QShortcut::activated, this, [this]() {
        showPlugin(PLUGIN_TYPE_SEARCHBAR);
    });

    // zoom_in
    connect(createNewShotcut("shortcuts.terminal.zoom_in"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentPage();
        if (page) {
            page->zoomInCurrentTierminal();
        }
        /******** Add by nt001000 renfeixiang 2020-05-20:增加 雷神窗口根据字体大小设置最小高度函数 Begin***************/
        setQuakeWindowMinHeight();
        /******** Add by nt001000 renfeixiang 2020-05-20:增加 雷神窗口根据字体大小设置最小高度函数 End***************/
    });

    // zoom_out
    connect(createNewShotcut("shortcuts.terminal.zoom_out"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentPage();
        if (page) {
            page->zoomOutCurrentTerminal();
        }
        /******** Add by nt001000 renfeixiang 2020-05-20:增加 雷神窗口根据字体大小设置最小高度函数 Begin***************/
        setQuakeWindowMinHeight();
        /******** Add by nt001000 renfeixiang 2020-05-20:增加 雷神窗口根据字体大小设置最小高度函数 End***************/
    });

    // default_size
    connect(createNewShotcut("shortcuts.terminal.default_size"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentPage();
        if (page) {
            page->setFontSize(Settings::instance()->fontSize());
        }
        /******** Add by nt001000 renfeixiang 2020-05-20:增加 雷神窗口根据字体大小设置最小高度函数 Begin***************/
        setQuakeWindowMinHeight();
        /******** Add by nt001000 renfeixiang 2020-05-20:增加 雷神窗口根据字体大小设置最小高度函数 End***************/
    });

    // select_all
    connect(createNewShotcut("shortcuts.terminal.select_all"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentPage();
        if (page) {
            qDebug() << "selectAll";
            page->selectAll();
        }
    });

    // switch_fullscreen
    connect(createNewShotcut("shortcuts.advanced.switch_fullscreen"), &QShortcut::activated, this, [this]() {
        switchFullscreen();
    });

    // rename_tab
    connect(createNewShotcut("shortcuts.advanced.rename_title"), &QShortcut::activated, this, [this]() {
        showPlugin(PLUGIN_TYPE_NONE);
        TermWidgetPage *page = currentPage();
        if (page) {
            QString currTabTitle = m_tabbar->tabText(m_tabbar->currentIndex());
            page->showRenameTitleDialog(currTabTitle);
        }
    });

    // display_shortcuts
    connect(createNewShotcut("shortcuts.advanced.display_shortcuts"), &QShortcut::activated, this, [this]() {
        qDebug() << "displayShortcuts";
        displayShortcuts();
    });

    // custom_command
    connect(createNewShotcut("shortcuts.advanced.custom_command"), &QShortcut::activated, this, [this]() {
        if (m_CurrentShowPlugin == PLUGIN_TYPE_CUSTOMCOMMAND) {
            showPlugin(PLUGIN_TYPE_NONE);
        } else {
            showPlugin(PLUGIN_TYPE_CUSTOMCOMMAND);
        }
    });

    // remote_management
    connect(createNewShotcut("shortcuts.advanced.remote_management"), &QShortcut::activated, this, [this]() {
        if (m_CurrentShowPlugin == PLUGIN_TYPE_REMOTEMANAGEMENT) {
            showPlugin(PLUGIN_TYPE_NONE);
        } else {
            showPlugin(PLUGIN_TYPE_REMOTEMANAGEMENT);
        }
    });
    /********************* Modify by n014361 wangpeili End ************************/

    for (int i = 1; i <= 9; i++) {
        QString shortCutStr = QString("alt+%1").arg(i);
        qDebug() << shortCutStr;
        QShortcut *switchTabSC = new QShortcut(QKeySequence(shortCutStr), this);
        connect(switchTabSC, &QShortcut::activated, this, [this, i]() {
            TermWidgetPage *page = currentPage();
            if (page) {
                if (9 == i && m_tabbar->count() > 9) {
                    m_tabbar->setCurrentIndex(m_tabbar->count() - 1);
                    return;
                }

                if (i - 1 >= m_tabbar->count()) {
                    qDebug() << "i - 1 > tabcount" << i - 1 << m_tabbar->count() << endl;
                    return;
                }

                qDebug() << "index" << i - 1 << endl;
                m_tabbar->setCurrentIndex(i - 1);
                return;
            }
            qDebug() << "currentPage nullptr ??";
        });
    }
}

void MainWindow::initConnections()
{
    connect(this, &MainWindow::mainwindowClosed, WindowsManager::instance(), &WindowsManager::onMainwindowClosed);
    connect(Settings::instance(), &Settings::windowSettingChanged, this, &MainWindow::onWindowSettingChanged);
    connect(Settings::instance(), &Settings::shortcutSettingChanged, this, &MainWindow::onShortcutSettingChanged);
    connect(this, &MainWindow::newWindowRequest, this, &MainWindow::onCreateNewWindow);
    connect(DApplicationHelper::instance(), &DApplicationHelper::themeTypeChanged, this, [ = ]() {
        //变成自动变色的图标以后，不需要来回变了。
        // applyTheme();
    });
    connect(qApp, &QGuiApplication::applicationStateChanged, this, &MainWindow::onApplicationStateChanged);
}


void MainWindow::handleTitleBarMenuFocusPolicy()
{
    QLayout *layout = titlebar()->layout();
    for (int i = 0; i < layout->count(); ++i) {
        QWidget *widget = layout->itemAt(i)->widget();
        if (widget != nullptr && QString(widget->metaObject()->className()) ==  QString("QWidget")) {
            QLayout *widgetLayout = widget->layout();
            for (int j = 0; j < widgetLayout->count(); ++j) {
                QWidget *widget = widgetLayout->itemAt(j)->widget();
                if (widget != nullptr && QString(widget->metaObject()->className()) ==  QString("QWidget")) {
                    QLayout *wLayout = widget->layout();
                    for (int k = 0; k < wLayout->count(); ++k) {
                        QWidget *widget = wLayout->itemAt(k)->widget();
                        if (widget != nullptr && QString(widget->metaObject()->className()).contains("Button")) {
                            widget->setFocusPolicy(Qt::NoFocus);
                        }
                    }
                }
            }
        }
    }
}

int MainWindow::executeCMD(const char *cmd)
{
    char *result;
    char buf_ps[1024] = {0};
    FILE *ptr;

    ptr = popen(cmd, "r");
    result = fgets(buf_ps, 1024, ptr);

    QString qStr(result);
    int num = qStr.toInt() ;
    pclose(ptr);
    return  num;
}

/*******************************************************************************
 1. @函数:    showPlugin
 2. @作者:    n014361 王培利
 3. @日期:    2020-03-26
 4. @说明:   由mainwindow统一指令当前显示哪个插件
*******************************************************************************/
void MainWindow::showPlugin(const QString &name)
{
    m_CurrentShowPlugin = name;
    if (name != PLUGIN_TYPE_NONE) {
        qDebug() << "show Plugin" << name;
    }

    emit showPluginChanged(name);
}
/*******************************************************************************
 1. @函数:    hidePlugin
 2. @作者:    n014361 王培利
 3. @日期:    2020-04-22
 4. @说明:    快速隐藏所有插件，resize专用的。
*******************************************************************************/
void MainWindow::hidePlugin()
{
    if (m_CurrentShowPlugin == PLUGIN_TYPE_NONE) {
        return;
    }
    qDebug() << "hide Plugin" << m_CurrentShowPlugin;
    m_CurrentShowPlugin = PLUGIN_TYPE_NONE;
    emit quakeHidePlugin();
}

QString MainWindow::selectedText(bool preserveLineBreaks)
{
    TermWidgetPage *page = currentPage();
    if (page) {
        if (page->currentTerminal()) {
            return page->currentTerminal()->selectedText(preserveLineBreaks);
        }
    }
    qDebug() << "not point terminal??";
    return  "";
}

void MainWindow::onCreateNewWindow(QString workingDir)
{
    Q_UNUSED(workingDir);
    // 调用一个新的进程，开启终端
    QProcess process;
    process.startDetached(QCoreApplication::applicationFilePath());
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    //这里调试用
    if ((event->type() != QEvent::Paint)
            && (event->type() != QEvent::MetaCall)
            && (event->type() != QEvent::UpdateRequest)
            && (event->type() != QEvent::LayoutRequest)
            && (event->type() != QEvent::Timer)
            && (event->type() != QEvent::Leave)
            && (event->type() != QEvent::HoverLeave)
            && (event->type() != QEvent::CursorChange)
            && (event->type() != QEvent::MouseMove)
            && (event->type() != QEvent::LanguageChange)
            && (event->type() != QEvent::PolishRequest)
            && (event->type() != QEvent::DynamicPropertyChange)
            && (event->type() != QEvent::Resize)
            && (event->type() != QEvent::MouseButtonPress)
            && (event->type() != QEvent::UpdateLater)
            && (event->type() != QEvent::StatusTip)
            && (event->type() != QEvent::DeferredDelete)
            && (event->type() != QEvent::Polish)
            && (event->type() != QEvent::UpdateLater)
            && (event->type() != QEvent::HoverMove)
            && (event->type() != QEvent::Enter)
            && (event->type() != QEvent::KeyRelease)
            && (event->type() != QEvent::HoverEnter)
            && (event->type() != QEvent::MouseButtonRelease)
            && (event->type() != QEvent::HoverEnter)
            && (event->type() != QEvent::WindowDeactivate)
            && (event->type() != QEvent::InputMethodQuery)) {
        //qDebug() << "event" << event->type() << watched;
    }

    if (event->type() == QEvent::KeyPress) {
        TermWidget *term = currentPage()->currentTerminal();
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            if (term->enterSzCommand()) {
                //--added by qinyaning(nyq) to slove Unable to download file from server, time: 2020.4.13 18:21--//
                pressEnterKey("\nsz \"${files[@]}\"");
                //-------------------------------------
                executeDownloadFile();
                term->setEnterSzCommand(false);
            }
        }
        if ((keyEvent->modifiers() == Qt::ControlModifier) && (keyEvent->key() == Qt::Key_C || keyEvent->key() == Qt::Key_D)) {
            if (term->enterSzCommand()) {
                term->setEnterSzCommand(false);
            }
        }
    }

    return DMainWindow::eventFilter(watched, event);
}


/*******************************************************************************
 1. @函数:    onSettingValueChanged
 2. @作者:    n014361 王培利
 3. @日期:    2020-02-19
 4. @说明:    参数修改统一接口
*******************************************************************************/
void MainWindow::onWindowSettingChanged(const QString &keyName)
// void MainWindow::onSettingValueChanged(const int &keyIndex, const QVariant &value)
{
    if (keyName == "advanced.window.blurred_background") {
        setEnableBlurWindow(Settings::instance()->backgroundBlur());
        return;
    }

    // use_on_starting重启生效
    if (keyName == "advanced.window.use_on_starting") {
        QString state = Settings::instance()->settings->option("advanced.window.use_on_starting")->value().toString();
        if ("window_normal" == state) {
            m_IfUseLastSize = true;
            /******** Modify by nt001000 renfeixiang 2020-05-25: 文件wininfo-config.conf中参数,使用定义更换window_width，window_height Begin***************/
            m_winInfoConfig->setValue(CONFIG_WINDOW_WIDTH, 1000);
            m_winInfoConfig->setValue(CONFIG_WINDOW_HEIGHT, 600);
            /******** Modify by nt001000 renfeixiang 2020-05-25: 文件wininfo-config.conf中参数,使用定义更换window_width，window_height End***************/
        } else {
            m_IfUseLastSize = false;
        }
        qDebug() << "settingValue[" << keyName << "] changed to " << state
                 << ", auto effective when next start!";
        return;
    }
    // auto_hide_raytheon_window在使用中自动读取生效
    if ((keyName == "advanced.window.auto_hide_raytheon_window") || (keyName == "advanced.window.use_on_starting")) {
        qDebug() << "settingValue[" << keyName << "] changed to " << Settings::instance()->OutputtingScroll()
                 << ", auto effective when happen";
        onApplicationStateChanged(QApplication::applicationState());
        return;
    }

    qDebug() << "settingValue[" << keyName << "] changed is not effective";
}

void MainWindow::onShortcutSettingChanged(const QString &keyName)
{
    qDebug() << "Shortcut[" << keyName << "] changed";
    if (m_BuiltInShortcut.contains(keyName)) {
        QString value = Settings::instance()->settings->option(keyName)->value().toString();
        m_BuiltInShortcut[keyName]->setKey(QKeySequence(value));
        return;
    }

    qDebug() << "Shortcut[" << keyName << "] changed is unknown!";
}

void MainWindow::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event);
}

/*******************************************************************************
 1. @函数:    mousePressEvent
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-04-28
 4. @说明:    左键点击隐藏插件
*******************************************************************************/
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        showPlugin(PLUGIN_TYPE_NONE);
    }
}

void MainWindow::setNewTermPage(TermWidgetPage *termPage, bool activePage)
{
    m_termStackWidget->addWidget(termPage);
    if (activePage) {
        m_termStackWidget->setCurrentWidget(termPage);
    }
}

void MainWindow::createNewWorkspace()
{
    addTab(currentPage()->createCurrentTerminalProperties(), true);
}
/*******************************************************************************
 1. @函数:    applyTheme
 2. @作者:    n014361 王培利
 3. @日期:    2020-03-09
 4. @说明:    非DTK控件手动匹配系统主题的修改
*******************************************************************************/
void MainWindow::applyTheme()
{
    // m_exitFullScreen->setIcon(QIcon::fromTheme("dt_exit_fullscreen"));
    return;
}

ShortcutManager *MainWindow::getShortcutManager()
{
    return m_shortcutManager;
}

MainWindowPluginInterface *MainWindow::getPluginByName(const QString &name)
{
    for (int i = 0; i < m_plugins.count(); i++) {
        if (m_plugins.at(i)->getPluginName() == name) {
            return m_plugins.at(i);
        }
    }
    return nullptr;
}

/*******************************************************************************
 1. @函数:void MainWindow::displayShortcuts()
 2. @作者:     n014361 王培利
 3. @日期:     2020-01-10
 4. @说明:     显示快捷键列表信息
*******************************************************************************/
void MainWindow::displayShortcuts()
{
    QRect rect = window()->geometry();
    //--added by qinyaning(nyq) to solve the problem of can't show center--//
    if (m_isQuakeWindow)
        rect = QApplication::desktop()->availableGeometry();
    //---------------------------------------------------------------------//
    QPoint pos(rect.x() + rect.width() / 2, rect.y() + rect.height() / 2);

    QJsonArray jsonGroups;
    createJsonGroup("terminal", jsonGroups);
    createJsonGroup("workspace", jsonGroups);
    createJsonGroup("advanced", jsonGroups);
    QJsonObject shortcutObj;
    shortcutObj.insert("shortcut", jsonGroups);

    QJsonDocument doc(shortcutObj);

    QStringList shortcutString;
    QString param1 = "-j=" + QString(doc.toJson().data());
    QString param2 = "-p=" + QString::number(pos.x()) + "," + QString::number(pos.y());
    shortcutString << param1 << param2;

    QProcess *shortcutViewProcess = new QProcess();
    shortcutViewProcess->startDetached("deepin-shortcut-viewer", shortcutString);

    connect(shortcutViewProcess, SIGNAL(finished(int)), shortcutViewProcess, SLOT(deleteLater()));
}

/*******************************************************************************
 1. @函数: void MainWindow::createJsonGroup(const QString &keyCategory, QJsonArray &jsonGroups)
 2. @作者:     n014361 王培利
 3. @日期:     2020-01-10
 4. @说明:     创建JsonGroup组
*******************************************************************************/
void MainWindow::createJsonGroup(const QString &keyCategory, QJsonArray &jsonGroups)
{
    qDebug() << keyCategory;

    QString strGroupName = "";
    if (keyCategory == "workspace") {
        strGroupName =  tr("Workspace");
    } else if (keyCategory == "terminal") {
        strGroupName =  tr("Terminal");
    } else if (keyCategory == "advanced") {
        strGroupName =  tr("Advanced");
    } else {
        return;
    }
    QString groupname = "shortcuts." + keyCategory;

    QJsonArray JsonArry;
    for (auto opt :
            Settings::instance()->settings->group(groupname)->options()) {  // Settings::instance()->settings->keys())
        QJsonObject jsonItem;
        QString name = QObject::tr(opt->name().toUtf8().data());
        /***del begin by ut001121 zhangmeng 修复BUG#23269 快捷键菜单“切换全屏”显示与文案不一致***/
        /*if (opt->name() == "Fullscreen")
            name = tr("Toggle fullscreen");
        */
        /***del end by ut001121 zhangmeng***/
        jsonItem.insert("name", name);
        jsonItem.insert("value", opt->value().toString());
        JsonArry.append(jsonItem);
    }

    if (keyCategory == "workspace") {
        QJsonObject jsonItem;
        jsonItem.insert("name", tr("Select workspace"));
        jsonItem.insert("value", "Alt+1~9");
        JsonArry.append(jsonItem);
    }

    QJsonObject JsonGroup;
    JsonGroup.insert("groupName", strGroupName);
    JsonGroup.insert("groupItems", JsonArry);
    jsonGroups.append(JsonGroup);
}
/*******************************************************************************
 1. @函数:    createNewShotcut
 2. @作者:    n014361 王培利
 3. @日期:    2020-02-20
 4. @说明:    创建内置快捷键管理,
             AutoRepeat 是否可以连续触发快捷键, 默认可以
*******************************************************************************/
QShortcut *MainWindow::createNewShotcut(const QString &key, bool AutoRepeat)
{
    QString value = Settings::instance()->settings->option(key)->value().toString();
    QShortcut *shortcut = new QShortcut(QKeySequence(value), this);
    m_BuiltInShortcut[key] = shortcut;
    shortcut->setAutoRepeat(AutoRepeat);
    // qDebug() << "createNewShotcut" << key << value;
    return shortcut;
}


/**
 * Upload file to remote server
 */
void MainWindow::remoteUploadFile()
{
    QStringList fileName = Utils::showFilesSelectDialog(this);
    if (!fileName.isEmpty()) {
        pressCtrlAt();
        sleep(100);
        QString strTxt = "sz ";
        for (QString str : fileName) {
            strTxt += str + " ";
        }
        currentPage()->sendTextToCurrentTerm(strTxt);
        currentPage()->sendTextToCurrentTerm("\n");
    }
}

/**
 * Download file from remote server
 */
void MainWindow::remoteDownloadFile()
{
    TermWidget *term = currentPage()->currentTerminal();
    downloadFilePath = Utils::showDirDialog(this);

    if (!downloadFilePath.isNull() && !downloadFilePath.isEmpty()) {
        //QString strTxt = "read -e -a files -p \"" + tr("Type path to download file") + ": \"; sz \"${files[@]}\"\n";
        //currentTab()->sendTextToCurrentTerm(strTxt);
        //--added by qinyaning(nyq) to slove Unable to download file from server, time: 2020.4.13 18:21--//
        QString strTxt = QString("read -e -a files -p \"%1: \"").arg(tr("Type path to download file"));
        pressEnterKey(strTxt);
        currentPage()->sendTextToCurrentTerm("\n");
        //-------------------
        term->setEnterSzCommand(true);
        //sleep(100);//
    }
}
/*******************************************************************************
 1. @函数:    onApplicationStateChanged
 2. @作者:    n014361 王培利
 3. @日期:    2020-04-28
 4. @说明:    当雷神窗口处于非激动状态自动隐藏
*******************************************************************************/
void MainWindow::onApplicationStateChanged(Qt::ApplicationState state)
{
    g_appState = state;
    return;
    qDebug() << "Application  state " << state << isActiveWindow() << isVisible() << windowState();
    //这块有问题需要改
    return;

    // 这个逻辑的针对的是在有弹窗情况下，windows+D后，可以让弹窗和主窗口一起弹出．
    // 如果激活应用，就激活主窗口，可以让弹窗和主窗口一起弹出．注意：不能传给弹窗父指针！！
    if (state == Qt::ApplicationActive) {
        activateWindow();
        return;
    }

    // 下面的代码是雷神窗口自动隐藏功能．
    // 不是雷神窗口，不管
    if (!m_isQuakeWindow) {
        return;
    }
    // 开关没开，不管
    if (!Settings::instance()->settings->option("advanced.window.auto_hide_raytheon_window")->value().toBool()) {
        return;
    }

    // 主窗口因为有弹窗而未激活，不管．
    if (this != QApplication::activeWindow() && QApplication::activeWindow() != nullptr) {
        qDebug() << QApplication::activeWindow();
        return;
    }

    // 激活应用的指令传不到这里．传到这里的都是非激活指令．
    hide();

    qDebug() << "Application not Active,　now hide" << state;
}

/**
 * after sz command,wait input file and download file.
 */
void MainWindow::executeDownloadFile()
{
    //--modified by qinyaning(nyq) to slove Unable to download file from server, time: 2020.4.13 18:21--//
    currentPage()->sendTextToCurrentTerm("\r\n");
    sleep(1000);
    pressCtrlAt();
    sleep(100);
    QString strCd = "cd " + downloadFilePath;
    currentPage()->sendTextToCurrentTerm(strCd);
    //sleep(100);
    QString strRz = "\r\nrz -be";
    currentPage()->sendTextToCurrentTerm(strRz);
    downloadFilePath = "";
    //-------------------------------------------
}

void MainWindow::pressCtrlAt()
{
    QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_At, Qt::ControlModifier);
    QApplication::sendEvent(focusWidget(), &keyPress);
}

void MainWindow::pressCtrlU()
{
    QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_U, Qt::ControlModifier);
    QApplication::sendEvent(focusWidget(), &keyPress);
}

void MainWindow::pressCtrlC()
{
    QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_C, Qt::ControlModifier);
    QApplication::sendEvent(focusWidget(), &keyPress);
}

void MainWindow::sleep(unsigned int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while (QTime::currentTime() < dieTime) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
}

QList<MainWindow *> MainWindow::getWindowList()
{
    return m_windowList;
}
//--added by qinyaning(nyq) to slove Unable to download file from server, time: 2020.4.13 18:21--//
void MainWindow::pressEnterKey(const QString &text)
{
    QKeyEvent event(QEvent::KeyPress, 0, Qt::NoModifier, text);
    QApplication::sendEvent(focusWidget(), &event);  // expose as a big fat keypress event
}

int MainWindow::getDesktopIndex() const
{
    return m_desktopIndex;
}

/*******************************************************************************
 1. @函数:    setQuakeWindowMinHeight
 2. @作者:    ut001000 任飞翔
 3. @日期:    2020-05-20
 4. @说明:    雷神窗口根据字体和字体大小设置最小高度
*******************************************************************************/
/******** Add by nt001000 renfeixiang 2020-05-20:增加雷神窗口根据字体和字体大小设置最小高度函数 Begin***************/
void MainWindow::setQuakeWindowMinHeight()
{
    if (isQuakeMode()) {
        int height = 0;
        QFontMetrics fm(currentPage()->currentTerminal()->getTerminalFont());
        height = fm.height();
        height = 60 + height * 2;
        setMinimumHeight(height);
    }
}
/******** Add by nt001000 renfeixiang 2020-05-20:增加雷神窗口根据字体和字体大小设置最小高度函数 End***************/

void MainWindow::changeEvent(QEvent *event)
{
    // 不是激活事件,不处理
    if (event->type() == QEvent::ActivationChange) {
        onAppFocusChangeForQuake();
    }

    // 雷神窗口没有其它需要调整的
    if (m_isQuakeWindow) {
        QMainWindow::changeEvent(event);
        return;
    }

    if (m_exitFullScreen) {
        bool isFullscreen = window()->windowState().testFlag(Qt::WindowFullScreen);
        m_exitFullScreen->setVisible(isFullscreen);
        titlebar()->setMenuVisible(!isFullscreen);
        titlebar()->findChild<QWidget *>("DTitlebarDWindowQuitFullscreenButton")->hide();
    }

    QMainWindow::changeEvent(event);
}

/*******************************************************************************
 1. @函数:    onAppFocusChangeForQuake
 2. @作者:    ut001121 张猛
 3. @日期:    2020-05-22
 4. @说明:    处理雷神窗口丢失焦点自动隐藏功能
*******************************************************************************/
void MainWindow::onAppFocusChangeForQuake()
{
    // 开关关闭，不处理
    if (!Settings::instance()->settings->option("advanced.window.auto_hide_raytheon_window")->value().toBool()) {
        return;
    }

    // 设置框或其他弹框弹出,不处理
    if (Service::instance()->isSettingDialogVisible() || Service::instance()->getIsDialogShow()) {
        return;
    }

    // 不是雷神窗口，不处理
    if (!m_isQuakeWindow) {
        // 非雷神窗口被激活,意味着雷神窗口要被隐藏
        MainWindow *pQuakeWindow = WindowsManager::instance()->getQuakeWindow();
        if (isActiveWindow() && pQuakeWindow && pQuakeWindow->isVisible()) {
            pQuakeWindow->hide();
        }
        return;
    }

    // 处于激活状态,不处理
    if (isActiveWindow()) {
        return;
    }

//    // 获取应用程序活动窗口
//    QWidget *pWidget = QApplication::activeWindow();

//    // 应用程序活动窗口不是MainWindow窗口,不处理
//    if (pWidget && pWidget->metaObject()->className() != QStringLiteral("MainWindow")) {
//        return;
//    }

    // 隐藏雷神窗口
    hide();
}
