// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"
#include "mainwindowplugininterface.h"
#include "settings.h"
#include "shortcutmanager.h"
#include "tabbar.h"
#include "termproperties.h"
#include "termwidgetpage.h"
#include "termwidget.h"
#include "titlebar.h"
#include "encodepanelplugin.h"
#include "customcommandplugin.h"
#include "serverconfigmanager.h"
#include "utils.h"
#include "dbusmanager.h"
#include "windowsmanager.h"
#include "service.h"
#include "terminalapplication.h"
#include "switchthememenu.h"

#include <DSettings>
#include <DSettingsGroup>
#include <DSettingsOption>
#include <DSettingsWidgetFactory>
#include <DThemeManager>
#include <DTitlebar>
#include <DFileDialog>
#include <DAboutDialog>
#include <DImageButton>
#include <DLog>
#include <DWindowManagerHelper>

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
#include <QScreen>

#include <fstream>

using std::ifstream;
using std::ofstream;

/******** Add by ut001000 renfeixiang 2020-08-07:将最小宽度和高度设置成全局变量***************/
const int MainWindow::m_MinWidth = WINDOW_MIN_WIDTH;
const int MainWindow::m_MinHeight = WINDOW_MIN_HEIGHT;


//qt5.15里，需要补发事件,实现雷神终端的正常隐藏。bug#85079
static QList<QEvent *> DeActivationChangeEventList;

DWIDGET_USE_NAMESPACE

// 定义雷神窗口边缘,接近边缘光标变化图标
#define QUAKE_EDGE 5

MainWindow::MainWindow(TermProperties properties, QWidget *parent)
    : DMainWindow(parent)
    , m_menu(new QMenu(this))
    , m_tabbar(nullptr)
    , m_centralWidget(new QWidget(this))
    , m_centralLayout(new QVBoxLayout(m_centralWidget))
    , m_termStackWidget(new QStackedWidget)
    , m_properties(properties)
    , m_isQuakeWindow(properties[QuakeMode].toBool())
    , m_winInfoConfig(new QSettings(getWinInfoConfigPath(), QSettings::IniFormat, this))
{
    /******** Add by ut001000 renfeixiang 2020-08-13:增加 Begin***************/
    m_menu->setObjectName("MainWindowQMenu");
    m_centralWidget->setObjectName("MainWindowCentralQWidget");
    m_centralLayout->setObjectName("MainWindowCentralLayout");
    m_termStackWidget->setObjectName("MainWindowTermQStackedWidget");
    m_winInfoConfig->setObjectName("MainWindowWindowQSettings");
    /******** Add by ut001000 renfeixiang 2020-08-13:增加 End***************/
    static int id = 0;
    m_MainWindowID = ++id;
    if (0 == m_ReferedAppStartTime) {
        // 主进程的启动时间存在APP中
        TerminalApplication *app = static_cast<TerminalApplication *>(qApp);
        m_ReferedAppStartTime = app->getStartTime();
        qInfo() << "[main app] Start Time = "
                << QDateTime::fromMSecsSinceEpoch(m_ReferedAppStartTime).toString("yyyy-MM-dd hh:mm:ss:zzz");
    } else {
        qInfo() << "[sub app] Start Time = "
                << QDateTime::fromMSecsSinceEpoch(m_ReferedAppStartTime).toString("yyyy-MM-dd hh:mm:ss:zzz");
    }
    m_CreateWindowTime = Service::instance()->getEntryTime();
    qInfo() << "MainWindow Create Time = "
            << QDateTime::fromMSecsSinceEpoch(m_CreateWindowTime).toString("yyyy-MM-dd hh:mm:ss:zzz");
}

void MainWindow::initUI()
{
    initWindow();
    // Plugin may need centralWidget() to work so make sure initPlugin() is after setCentralWidget()
    // Other place (eg. create titlebar menu) will call plugin method so we should create plugins before init other
    // parts.
    initPlugins();
    initTabBar();
    initTitleBar();
    initWindowAttribute();
    initFileWatcher();

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

inline void MainWindow::slotTabBarClicked(int index, QString tabIdentifier)
{
    //DTabBar在左右移动时右侧会出现空白，点击会导致crash，这里加个判断防止index出现非法的情况
    if (index < 0)
        return;

    TermWidgetPage *tabPage = qobject_cast<TermWidgetPage *>(m_termStackWidget->widget(index));
    TermWidget *term = tabPage->currentTerminal();
    bool bIdle = !(term->hasRunningProcess());

    if (bIdle && isTabChangeColor(tabIdentifier)) {
        m_tabVisitMap.insert(tabIdentifier, true);
        m_tabChangeColorMap.insert(tabIdentifier, false);
        m_tabbar->removeNeedChangeTextColor(tabIdentifier);
    }
}

inline void MainWindow::slotTabCurrentChanged(int index)
{
    const QString identifier = m_tabbar->identifier(index);
    focusPage(identifier);
    updateWindowTitle();
}

inline void MainWindow::slotTabAddRequested()
{
    createNewTab();
}

inline void MainWindow::slotTabCloseRequested(int index)
{

    closeTab(m_tabbar->identifier(index));
}

inline void MainWindow::slotMenuCloseTab(QString Identifier)
{
    closeTab(Identifier);
}

// TAB菜单发来的关闭其它窗口请求,需要逐一关闭
inline void MainWindow::slotMenuCloseOtherTab(QString Identifier)
{
    closeOtherTab(Identifier);
}

inline void MainWindow::slotShowRenameTabDialog(QString Identifier)
{
    // 获取标签选中的tab对应的标签标题
    getPageByIdentifier(Identifier)->showRenameTitleDialog();
}

void MainWindow::initTabBar()
{
    m_tabbar = new TabBar(this);
    m_tabbar->setFocusPolicy(Qt::NoFocus);

    connect(m_tabbar, &TabBar::tabBarClicked, this, &MainWindow::slotTabBarClicked);

    // 点击TAB上页触发
    connect(m_tabbar, &DTabBar::currentChanged, this, &MainWindow::slotTabCurrentChanged, Qt::QueuedConnection);

    // 点击TAB上的＂＋＂触发
    connect(m_tabbar, &DTabBar::tabAddRequested, this, &MainWindow::slotTabAddRequested, Qt::QueuedConnection);

    // 点击TAB上的＂X＂触发
    connect(m_tabbar, &DTabBar::tabCloseRequested, this, &MainWindow::slotTabCloseRequested, Qt::QueuedConnection);

    // TAB菜单发来的关闭请求
    connect(m_tabbar, &TabBar::menuCloseTab, this, &MainWindow::slotMenuCloseTab);

    // TAB菜单发来的关闭其它窗口请求,需要逐一关闭
    connect(m_tabbar, &TabBar::menuCloseOtherTab, this, &MainWindow::slotMenuCloseOtherTab);

    connect(m_tabbar, &TabBar::showRenameTabDialog, this, &MainWindow::slotShowRenameTabDialog);

    // 如果此时是拖拽的窗口，暂时先不添加tab(默认添加tab后会新建工作区)
    // 需要使用拖入/拖出标签对应的那个TermWidgetPage控件
    if (m_properties[DragDropTerminal].toBool())
        return;

    addTab(m_properties);
}

inline void MainWindow::slotOptionButtonPressed()
{
    showPlugin(PLUGIN_TYPE_NONE);
    // 判断是否超过最大数量限制
    QList<QAction *> actionList = m_menu->actions();
    for (auto item : actionList) {
        if (tr("New window") == item->text()) {
            // 菜单根据数量自动设置true or false
            item->setEnabled(Service::instance()->isCountEnable());
        }
    }
}

inline void MainWindow::slotClickNewWindowTimeout()
{
    // 创建新的窗口
    qInfo() << "menu click new window";

    TermWidgetPage *tabPage = currentPage();
    TermWidget *term = tabPage->currentTerminal();
    QString currWorkingDir = term->workingDirectory();
    emit newWindowRequest(currWorkingDir);
}

inline void MainWindow::slotNewWindowActionTriggered()
{
    // 等待菜单消失  此处影响菜单创建窗口的性能
    m_createTimer->start(50);
}

void MainWindow::initOptionButton()
{
    // 全屏退出按钮
    // DTK的全屏按钮不能满足UI要求，隐去DTK最右侧的全屏
    QWidget *dtkbutton = titlebar()->findChild<QWidget *>("DTitlebarDWindowQuitFullscreenButton");
    if (dtkbutton != nullptr)
        dtkbutton->hide();
    else
        qInfo() << "can not found DTitlebarDWindowQuitFullscreenButton in DTitlebar";

    // option button
    DIconButton *optionBtn = titlebar()->findChild<DIconButton *>("DTitlebarDWindowOptionButton");
    if (optionBtn != nullptr) {
        // mainwindow的设置按钮触发
        connect(titlebar()->findChild<DIconButton *>("DTitlebarDWindowOptionButton"), &DIconButton::pressed, this, &MainWindow::slotOptionButtonPressed);
    } else {
        qInfo() << "can not found DTitlebarDWindowOptionButton in DTitlebar";
    }
}

void MainWindow::initOptionMenu()
{
    titlebar()->setMenu(m_menu);
    /******** Modify by m000714 daizhengwen 2020-04-03: 新建窗口****************/
    // 防止创建新窗口的action被多次触发
    m_createTimer = new QTimer(this);
    // 设置定时器,等待菜单消失,触发一次,防止多次被触发
    m_createTimer->setSingleShot(true);
    QAction *newWindowAction(new QAction(tr("New window"), this));
    connect(m_createTimer, &QTimer::timeout, this, &MainWindow::slotClickNewWindowTimeout);
    connect(newWindowAction, &QAction::triggered, this, &MainWindow::slotNewWindowActionTriggered);
    m_menu->addAction(newWindowAction);
    /********************* Modify by m000714 daizhengwen End ************************/
    for (auto &plugin : m_plugins) {
        QAction *pluginMenu = plugin->titlebarMenu(this);
        // 取消Encoding插件的菜单展示
        if (plugin->getPluginName() == PLUGIN_TYPE_ENCODING)
            continue;

        if (pluginMenu)
            m_menu->addAction(pluginMenu);
    }

    QAction *settingAction(new QAction(tr("Settings"), this));
    m_menu->addAction(settingAction);
    m_menu->addSeparator();

    //禁掉dtk主题菜单
    titlebar()->setSwitchThemeMenuVisible(false);
    //增加主题菜单
    addThemeMenuItems();

    connect(settingAction, &QAction::triggered, Service::instance(), &Service::slotShowSettingsDialog);
}

inline void MainWindow::slotFileChanged()
{
    QFileSystemWatcher *fileWatcher = qobject_cast<QFileSystemWatcher *>(sender());
    if (nullptr == fileWatcher)
        return;
    emit  Service::instance()->hostnameChanged();
    //这句话去了之后filechanged信号只能触发一次
    fileWatcher->addPath(HOSTNAME_PATH);
}

void MainWindow::initFileWatcher()
{
    QFileSystemWatcher *fileWatcher  = new QFileSystemWatcher(this);
    fileWatcher->addPath(HOSTNAME_PATH);
    //bug 53565 ut000442 hostname被修改后，全部窗口触发一次修改标题
    connect(fileWatcher, &QFileSystemWatcher::fileChanged, this, &MainWindow::slotFileChanged);
}

void MainWindow::initPlugins()
{
    // Todo: real plugin loader and plugin support.
    EncodePanelPlugin *encodePlugin = new EncodePanelPlugin(this);
    encodePlugin->initPlugin(this);

    customCommandPlugin = new CustomCommandPlugin(this);
    customCommandPlugin->initPlugin(this);

    remoteManagPlugin = new RemoteManagementPlugin(this);
    remoteManagPlugin->initPlugin(this);

    m_plugins.append(encodePlugin);
    m_plugins.append(customCommandPlugin);
    m_plugins.append(remoteManagPlugin);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setDefaultLocation()
{
    resize(WINDOW_DEFAULT_SIZE);
    singleFlagMove();
}

void MainWindow::singleFlagMove()
{
    if (m_properties[SingleFlag].toBool()) {
        Dtk::Widget::moveToCenter(this);
        qInfo() << "SingleFlag move" ;
    }
}

bool MainWindow::isTabVisited(const QString &tabIdentifier)
{
    return m_tabVisitMap.value(tabIdentifier);
}

bool MainWindow::isTabChangeColor(const QString &tabIdentifier)
{
    return m_tabChangeColorMap.value(tabIdentifier);
}

void MainWindow::addTab(TermProperties properties, bool activeTab)
{
    qint64 startTime = QDateTime::currentMSecsSinceEpoch();
    //如果不允许新建标签，则返回
    if (!beginAddTab())
        return;

    TermWidgetPage *termPage = new TermWidgetPage(properties, this);

    // pageID存在 tab中，所以page增删改操作都要由tab发起。
    int index = m_tabbar->addTab(termPage->identifier(), termPage->getCurrentTerminalTitle());
    qInfo() << "addTab index" << index;
    endAddTab(termPage, activeTab, index, startTime);
}

void MainWindow::addTabWithTermPage(const QString &tabName, bool activeTab, bool isVirtualAdd, TermWidgetPage *page, int insertIndex)
{
    qint64 startTime = QDateTime::currentMSecsSinceEpoch();
    //如果不允许新建标签，则返回
    if (!beginAddTab())
        return;

    TermWidgetPage *termPage = page;

    if (-1 == insertIndex)
        insertIndex = m_tabbar->currentIndex() + 1;

    // pageID存在 tab中，所以page增删改操作都要由tab发起。
    int index = m_tabbar->insertTab(insertIndex, page->identifier(), tabName);
    m_tabbar->setTabText(termPage->identifier(), tabName);
    qInfo() << "insertTab index" << index;

    //拖拽过程中存在一种标签预览模式，此时不需要真实添加
    if (!isVirtualAdd) {
        termPage->setParentMainWindow(this);
        endAddTab(termPage, activeTab, index, startTime);
    }
}

bool MainWindow::beginAddTab()
{
    /***add by ut001121 zhangmeng 修复BUG#24452 点击“+”按钮新建工作区，自定义命令/编码/远程管理插件未消失***/
    showPlugin(PLUGIN_TYPE_NONE);

    if (WindowsManager::instance()->widgetCount() >= MAXWIDGETCOUNT) {
        // 没有雷神，且是雷神让通过，不然不让通过
        if (!(nullptr == WindowsManager::instance()->getQuakeWindow() && m_isQuakeWindow)) {
            // 非雷神窗口不得超过MAXWIDGETCOUNT
            qInfo() << "addTab failed, can't create number more than " << MAXWIDGETCOUNT;
            return false;
        }
    }

    return true;
}

inline void MainWindow::slotLastTermClosed(const QString &identifier)
{
    closeTab(identifier);
}

void MainWindow::endAddTab(TermWidgetPage *termPage, bool activeTab, int index, qint64 startTime)
{
    setNewTermPage(termPage, activeTab);

    if (activeTab)
        m_tabbar->setCurrentIndex(index);

    TermWidget *term = termPage->currentTerminal();
    m_tabbar->saveSessionIdWithTabIndex(term->getSessionId(), index);
    m_tabbar->saveSessionIdWithTabId(term->getSessionId(), termPage->identifier());
    connect(termPage, &TermWidgetPage::termTitleChanged, this, &MainWindow::onTermTitleChanged);
    connect(termPage, &TermWidgetPage::lastTermClosed, this, &MainWindow::slotLastTermClosed);

    connect(this, &MainWindow::showPluginChanged, termPage, &TermWidgetPage::slotShowPluginChanged);

    // 快速隐藏搜索框
    connect(this, &MainWindow::quakeHidePlugin, termPage, &TermWidgetPage::slotQuakeHidePlugin);

    connect(termPage->currentTerminal(), &TermWidget::termIsIdle, this, &MainWindow::onTermIsIdle);
    qint64 endTime = QDateTime::currentMSecsSinceEpoch();
    QString strNewTabTime = GRAB_POINT + LOGO_TYPE + CREATE_NEW_TAB_TIME + QString::number(endTime - startTime);
    qInfo() << qPrintable(strNewTabTime);
}

bool MainWindow::hasRunningProcesses()
{
    for (int i = 0, count = m_termStackWidget->count(); i < count; i++) {
        TermWidgetPage *tabPage = qobject_cast<TermWidgetPage *>(m_termStackWidget->widget(i));
        if (!tabPage)
            continue;
        /******** Modify by nt001000 renfeixiang 2020-05-28:修改 判断当前tab中是否有其它分屏正在执行 bug#28910 Begin***************/
        //没有校验当前tab中是否有其它正在执行的分屏
        if (tabPage->runningTerminalCount() != 0) {
            /******** Modify by nt001000 renfeixiang 2020-05-28:修改 判断当前tab中是否有其它分屏正在执行 End***************/
            qInfo() << "here are processes running in this terminal tab... " << tabPage->identifier() << endl;
            return true;
        } else {
            qInfo() << "no processes running in this terminal tab... " << tabPage->identifier() << endl;
        }
    }

    return false;
}

void MainWindow::closeTab(const QString &identifier, bool hasConfirmed)
{
    /***add by ut001121 zhangmeng 20200508 修复BUG#24457 点击标签栏“x”按钮，右键菜单关闭工作区，关闭其它工作区，自定义命令/编码/远程管理插件未消失***/
    showPlugin(PLUGIN_TYPE_NONE);

    TermWidgetPage *tabPage = getPageByIdentifier(identifier);
    if (nullptr == tabPage)
        return;

    // 关闭前必须要切换过去先
    if (m_tabbar->currentIndex() != m_tabbar->getIndexByIdentifier(identifier))
        m_tabbar->setCurrentIndex(m_tabbar->getIndexByIdentifier(identifier));

    // 默认每个窗口关闭都提示一次．
    if (!hasConfirmed && tabPage->runningTerminalCount() != 0) {
        showExitConfirmDialog(Utils::CloseType_Tab, tabPage->runningTerminalCount(), this);
        return;
    }
    qInfo() << "Tab closed" << identifier;
    m_tabVisitMap.remove(identifier);
    m_tabChangeColorMap.remove(identifier);
    m_tabbar->removeTab(identifier);
    m_termStackWidget->removeWidget(tabPage);
    tabPage->deleteLater();

    /******** Add by ut001000 renfeixiang 2020-08-07:关闭tab时改变大小，bug#41436***************/
    updateMinHeight();

    if (m_tabbar->count() != 0) {
        updateTabStatus();
        focusCurrentPage();
        return;
    }
    qInfo() << "mainwindow close";
    close();
}

TermWidgetPage *MainWindow::getTermPage(const QString &identifier)
{
    return m_termWidgetPageMap.value(identifier);
}

void MainWindow::removeTermWidgetPage(const QString &identifier, bool isDelete)
{
    if (m_termWidgetPageMap.contains(identifier)) {
        TermWidgetPage *termPage = m_termWidgetPageMap.value(identifier);
        m_termStackWidget->removeWidget(termPage);
        m_termWidgetPageMap.remove(identifier);
        if (isDelete)
            delete termPage;
    }

    // 当所有tab标签页都关闭时，关闭整个MainWindow窗口
    if (m_termWidgetPageMap.isEmpty()) {
        qInfo() << "removeTermWidgetPage mainwindow close";
        /******** Modify by ut000438 王亮 2020-11-23:fix bug 55552:拖动标签页移动窗口过程中异常最大化 ***************/
        //当拖动标签过程中销毁窗口时，不保存销毁的窗口大小
        m_IfUseLastSize = false;
        close();
    }
}

void MainWindow::updateTabStatus()
{
    for (int i = 0; i < m_tabbar->count(); i++) {
        TermWidgetPage *tabPage = qobject_cast<TermWidgetPage *>(m_termStackWidget->widget(i));
        TermWidget *term = tabPage->currentTerminal();
        bool bIdle = !(term->hasRunningProcess());
        QString tabIdentifier = tabPage->identifier();

        if (bIdle) {
            if (isTabVisited(tabIdentifier)) {
                m_tabVisitMap.insert(tabIdentifier, false);
                m_tabChangeColorMap.insert(tabIdentifier, false);
                m_tabbar->removeNeedChangeTextColor(tabIdentifier);
            } else if (isTabChangeColor(tabIdentifier)) {
                m_tabbar->setChangeTextColor(tabIdentifier);
            } else {
                m_tabbar->removeNeedChangeTextColor(tabIdentifier);
            }
        } else {
            m_tabbar->removeNeedChangeTextColor(tabIdentifier);
        }
    }
}

QString MainWindow::getCurrTabTitle()
{
    return m_tabbar->tabText(m_tabbar->currentIndex());
}

void MainWindow::remoteUploadFile(QString strFileNames)
{
    pressCtrlAt();
    sleep(100);
    currentPage()->sendTextToCurrentTerm(strFileNames);
    currentPage()->sendTextToCurrentTerm("\n");
}

bool MainWindow::isFocusOnList()
{
    bool isFocus = true;
    DIconButton *addButton = m_tabbar->findChild<DIconButton *>("AddButton");
    // 判断是否找到
    if (addButton != nullptr) {
        // 判断按钮是否有焦点
        if (addButton->hasFocus()) {
            isFocus = false;
            qInfo() << "focus on AddButton";
        }
    } else {
        qInfo() << "can not found AddButton in DIconButton";
    }

    QList<QString> buttonList = {"DTitlebarDWindowOptionButton", "DTitlebarDWindowMinButton", "DTitlebarDWindowMaxButton", "DTitlebarDWindowCloseButton"};
    for (const QString &objectName : buttonList) {
        DIconButton *button = titlebar()->findChild<DIconButton *>(objectName);
        // 判断是否找到
        if (button != nullptr) {
            // 判断按钮是否有焦点
            if (button->hasFocus()) {
                isFocus = false;
                qInfo() << "focus on " << objectName;
            }
        } else {
            qInfo() << "can not found objectName in DIconButton";
        }
    }
    qInfo() << "focus on list : " << isFocus;
    return isFocus;

}

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

    if ((!hasConfirmed) && (runningCount != 0)) {
        // 全部关闭时，仅提示一次．
        showExitConfirmDialog(Utils::CloseType_OtherTab, runningCount, this);
        return;
    }

    // 关闭其它窗口，需要检测
    for (QString &id : closeTabIdList) {
        closeTab(id, true);
        qInfo() << " close" << id;
    }

    //如果是不关闭当前页的，最后回到当前页来．
    m_tabbar->setCurrentIndex(m_tabbar->getIndexByIdentifier(identifier));

    return;
}

void MainWindow::closeAllTab()
{
    QList<QString> closeTabIdList;
    for (int i = 0, count = m_termStackWidget->count(); i < count; i++) {
        TermWidgetPage *tabPage = qobject_cast<TermWidgetPage *>(m_termStackWidget->widget(i));
        closeTabIdList.append(tabPage->identifier());
    }

    // 全部关闭时，不再检测了，
    for (QString &id : closeTabIdList) {
        closeTab(id, true);
        qInfo() << " close" << id;
    }

    return;
}

inline void MainWindow::slotDDialogFinished(int result)
{
    OnHandleCloseType(result, Utils::CloseType(qobject_cast<DDialog *>(sender())->property("type").toInt()));
    /******** Modify by ut000439 wangpeili 2020-07-27:  bug 39643  ****************/
    if (result != 1 && qobject_cast<DDialog *>(sender())->property("focusCloseBtn").toBool())        {
        DIconButton *closeBtn = titlebar()->findChild<DIconButton *>("DTitlebarDWindowCloseButton");
        if (closeBtn != nullptr) {
            closeBtn->setFocus();
            qInfo() << "close button setFocus";
        } else {
            qInfo() << "can not found DTitlebarDWindowCloseButton in DTitlebar";
        }
    }
    /********************* Modify by n014361 wangpeili End ************************/
}

void MainWindow::showExitConfirmDialog(Utils::CloseType type, int count, QWidget *parent)
{
    // count < 1 不提示
    if (count < 1)
        return;

    QString title;
    QString txt;
    Utils::CloseType temtype = type;
    // 关闭窗口的时候，如果只有一个tab,提示的内容要为终端．
    if (Utils::CloseType_Window == type && 1 == m_tabbar->count())
        temtype = Utils::CloseType_Terminal;

    Utils::getExitDialogText(temtype, title, txt, count);

    /******** Modify by ut000439 wangpeili 2020-07-27:  bug 39643  ****************/
    // 关闭确认窗口前确认焦点位置是否在close button上，并且发起了关闭窗口
    bool closeBtnHasfocus = false;
    DIconButton *closeBtn = titlebar()->findChild<DIconButton *>("DTitlebarDWindowCloseButton");
    if ((closeBtn != nullptr) && closeBtn->hasFocus() && (Utils::CloseType_Window == type)) {
        closeBtnHasfocus = true;
        qInfo() << "before close window, focus widget is close button. ";
    }

    if (nullptr == closeBtn)
        qInfo() << "can not found DTitlebarDWindowCloseButton in DTitlebar";

    /********************* Modify by n014361 wangpeili End ************************/


    // 有弹窗显示
    Service::instance()->setIsDialogShow(this, true);

    DDialog *dlg = new DDialog(title, txt, parent);
    dlg->setIcon(QIcon::fromTheme("deepin-terminal"));
    dlg->addButton(QString(tr("Cancel", "button")), false, DDialog::ButtonNormal);
    /******** Modify by nt001000 renfeixiang 2020-05-21:修改Exit成Close Begin***************/
    dlg->addButton(QString(tr("Close", "button")), true, DDialog::ButtonWarning);
    /******** Modify by nt001000 renfeixiang 2020-05-21:修改Exit成Close End***************/
    dlg->setWindowModality(Qt::WindowModal);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->show();
    dlg->setProperty("type", type);

    /******** Modify by ut000439 wangpeili 2020-07-27:  bug 39643  ****************/
    if (closeBtnHasfocus)
        dlg->setProperty("focusCloseBtn",  true);
    else
        dlg->setProperty("focusCloseBtn",  false);

    /********************* Modify by n014361 wangpeili End ************************/

    /******** Modify by ut001000 renfeixiang 2020-06-03:修改 将dlg的槽函数修改为OnHandleCloseType，处理全部在OnHandleCloseType函数中 Begin***************/
    connect(dlg, &DDialog::finished, this, &MainWindow::slotDDialogFinished);

    return ;
}

void MainWindow::focusPage(const QString &identifier)
{
    TermWidgetPage *tabPage = getPageByIdentifier(identifier);
    if (tabPage) {
        //当切换焦点到PAGE页后，移除标签文字颜色
        m_tabbar->removeNeedChangeTextColor(identifier);
        m_termStackWidget->setCurrentWidget(tabPage);
        tabPage->focusCurrentTerm();
        return;
    }
    qInfo() << "focusTab nullptr identifier" << identifier;
}

void MainWindow::focusCurrentPage()
{
    focusPage(m_tabbar->identifier(m_tabbar->currentIndex()));
}

TermWidgetPage *MainWindow::currentPage()
{
    return qobject_cast<TermWidgetPage *>(m_termStackWidget->currentWidget());
}

TermWidgetPage *MainWindow::getPageByIdentifier(const QString &identifier)
{
    for (int i = 0, count = m_termStackWidget->count(); i < count; i++) {
        TermWidgetPage *tabPage = qobject_cast<TermWidgetPage *>(m_termStackWidget->widget(i));
        if (tabPage && tabPage->identifier() == identifier)
            return tabPage;
    }
    qInfo() << "getPageByIdentifier nullptr identifier" << identifier;
    return nullptr;
}

TermWidget *MainWindow::currentActivatedTerminal()
{
    if (currentPage())
        return currentPage()->currentTerminal();
    return nullptr;
}

void MainWindow::onTermIsIdle(QString tabIdentifier, bool bIdle)
{
    //如果标签被点过，移除标签颜色
    if (isTabVisited(tabIdentifier) && bIdle) {
        m_tabVisitMap.insert(tabIdentifier, false);
        m_tabChangeColorMap.insert(tabIdentifier, false);
        m_tabbar->removeNeedChangeTextColor(tabIdentifier);
        return;
    }

    if (bIdle) {
        //空闲状态如果标签被标记变色，则改变标签颜色
        if (m_tabbar->isNeedChangeTextColor(tabIdentifier)) {
            m_tabChangeColorMap.insert(tabIdentifier, true);
            m_tabbar->setChangeTextColor(tabIdentifier);
        }
    } else {

        //如果当前标签是活动标签，移除变色请求
        int activeTabIndex = m_tabbar->currentIndex();
        QString activeTabIdentifier = m_tabbar->tabData(activeTabIndex).toString();
        if (activeTabIdentifier == tabIdentifier) {
            m_tabVisitMap.insert(activeTabIdentifier, false);
            m_tabChangeColorMap.insert(activeTabIdentifier, false);
            m_tabbar->removeNeedChangeTextColor(activeTabIdentifier);
            return;
        }

        //标记变色，发起请求，稍后等空闲状态变色
        m_tabChangeColorMap.insert(tabIdentifier, false);
        DGuiApplicationHelper *appHelper = DGuiApplicationHelper::instance();
        DPalette pa = appHelper->standardPalette(appHelper->themeType());
        m_tabbar->setNeedChangeTextColor(tabIdentifier, pa.color(DPalette::Highlight));
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    //Add by ut001000 renfeixiang 2020-11-16 雷神动画效果时，不进行resize操作，直接return
    if (!isNotAnimation)
        return;

    // 保存窗口位置
    if (nullptr == resizeFinishedTimer) {
        resizeFinishedTimer = new QTimer(this);
        resizeFinishedTimer->setSingleShot(true);
        connect(resizeFinishedTimer, &QTimer::timeout, this, [this]() {
            saveWindowSize();
        });
    }
    //If the timer is already running, it will be stopped and restarted.
    resizeFinishedTimer->start(300);
    // 通知隐藏插件
    hidePlugin();

    DMainWindow::resizeEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveWindowSize();

    //fix bug: 64134 新建两个窗口，打开A窗口关于后关闭应用，B窗口打开关于，应用出现闪退
    TerminalApplication *app = static_cast<TerminalApplication *>(qApp);
    DAboutDialog *aboutDialog = app->aboutDialog();

    if (aboutDialog != nullptr) {
        disconnect(aboutDialog, &DAboutDialog::aboutToClose, nullptr, nullptr);

        //目前的关于对话框是非模态的,这里的处理是防止关于对话框可以打开多个的情况
        // 不能使用aboutToClose信号 应用能够打开多个的情况下 打开关于后直接关闭程序
        // 此时aboutToColose信号不会触发 再次打开程序并打开关于会出现访问野指针 程序崩溃的情况
        connect(aboutDialog, &DAboutDialog::destroyed, this, [ = ] {
            app->setAboutDialog(nullptr);
        });
    }
    //fix bug: 64134 end

    // 注销和关机时不需要确认了
    if (qApp->isSavingSession())
        DMainWindow::closeEvent(event);

    // 一页一页退出，当全部退出以后，mainwindow自然关闭．
    event->ignore();

    int runningCount = 0;
    QList<QString> closeTabIdList;
    int count = m_termStackWidget->count();
    for (int i = 0 ; i < count; i++) {
        TermWidgetPage *tabPage = qobject_cast<TermWidgetPage *>(m_termStackWidget->widget(i));
        closeTabIdList.append(tabPage->identifier());
        runningCount += tabPage->runningTerminalCount();
    }

    if ((!m_hasConfirmedClose) && (runningCount != 0)) {
        // 如果不能马上关闭，并且还在没有最小化．
        if ((runningCount != 0)  && isMinimized()) {
            qInfo() << "isMinimized........... " << endl;
            setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
        }

        // 全部关闭时，仅提示一次．
        showExitConfirmDialog(Utils::CloseType_Window, runningCount, this);
        return;
    }
    closeAllTab();


    if (0 == m_tabbar->count()) {
        DMainWindow::closeEvent(event);
        emit mainwindowClosed(this);
    }
}

bool MainWindow::isQuakeMode()
{
    return  m_isQuakeWindow;
}

void MainWindow::setIsQuakeWindowTab(bool isQuakeWindowTab)
{
    m_tabbar->setIsQuakeWindowTab(isQuakeWindowTab);
}

void MainWindow::onTermTitleChanged(QString title)
{
    TermWidgetPage *tabPage = qobject_cast<TermWidgetPage *>(sender());
    const bool customName = tabPage->property("TAB_CUSTOM_NAME_PROPERTY").toBool();
    if (!customName)
        m_tabbar->setTabText(tabPage->identifier(), title);
    updateWindowTitle();

    // 判定第一次修改标题的时候，认为终端已经创建成功
    // 以此认为第一次打开终端窗口结束，记录时间
    if (!hasCreateFirstTermialComplete) {
        Service::instance()->setMainTerminalIsStarted(true);
        firstTerminalComplete();
        hasCreateFirstTermialComplete = true;
    }
}

QString MainWindow::getConfigWindowState()
{
    QString windowState =
        Settings::instance()->settings->option("advanced.window.use_on_starting")->value().toString();

    // 启动参数配置的状态值优先于 内部配置的状态值
    if (m_properties.contains(StartWindowState)) {
        QString state = m_properties[StartWindowState].toString();
        qInfo() << "use line state set:" << state;
        if ("maximum" == state)
            windowState = "window_maximum";
        else if (state == "splitscreen")
            windowState = "split_screen";
        else if (state == "normal")
            windowState = "window_normal";
        else if (state == "fullscreen")
            windowState = state;
        else
            qInfo() << "error line state set:" << state << "ignore it!";
    }
    return  windowState;
}

QSize MainWindow::halfScreenSize()
{
    int w = qApp->desktop()->availableGeometry().width();
    int h = qApp->desktop()->availableGeometry().height();

    QSize size;
    //开启窗管特效时会有1px的border
    if (!DWindowManagerHelper::instance()->hasComposite())
        size = QSize(w / 2 - 2, h - 2 - 1);
    else
        size = QSize(w / 2, h - 1);
    return size;
}

QString MainWindow::getWinInfoConfigPath()
{
    QDir winInfoPath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    if (!winInfoPath.exists())
        winInfoPath.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));

    QString winInfoFilePath(winInfoPath.filePath("wininfo-config.conf"));
    return winInfoFilePath;
}

void MainWindow::initShortcuts()
{
    ShortcutManager::instance()->initConnect(this);
    connect(ShortcutManager::instance(), &ShortcutManager::addCustomCommandSignal, this, &MainWindow::addCustomCommandSlot);
    connect(ShortcutManager::instance(), &ShortcutManager::removeCustomCommandSignal, this, &MainWindow::removeCustomCommandSlot);

    /******** Modify by n014361 wangpeili 2020-01-10: 增加设置的各种快捷键修改关联***********×****/
    // new_tab 新建标签页
    connect(createNewShotcut("shortcuts.tab.new_tab", false), &QShortcut::activated, this, &MainWindow::slotShortcutNewTab);

    // close_tab 关闭标签页
    connect(createNewShotcut("shortcuts.tab.close_tab"), &QShortcut::activated, this, &MainWindow::slotShortcutCloseTab);

    // Close_other_tabs
    connect(createNewShotcut("shortcuts.tab.close_other_tabs"), &QShortcut::activated, this, &MainWindow::slotShortcutCloseOtherTabs);

    // previous_tab
    connect(createNewShotcut("shortcuts.tab.previous_tab"), &QShortcut::activated, this, &MainWindow::slotShortcutPreviousTab);

    // next_tab
    connect(createNewShotcut("shortcuts.tab.next_tab"), &QShortcut::activated, this, &MainWindow::slotShortcutNextTab);

    // horionzal_split
    connect(createNewShotcut("shortcuts.tab.horionzal_split", false), &QShortcut::activated, this, &MainWindow::slotShortcutHorizonzalSplit);

    // vertical_split
    connect(createNewShotcut("shortcuts.tab.vertical_split", false), &QShortcut::activated, this, &MainWindow::slotShortcutVerticalSplit);

    // select_upper_workspace
    connect(createNewShotcut("shortcuts.tab.select_upper_workspace"), &QShortcut::activated, this, &MainWindow::slotShortcutSelectUpperWorkspace);

    // select_lower_workspace
    connect(createNewShotcut("shortcuts.tab.select_lower_workspace"), &QShortcut::activated, this, &MainWindow::slotShortcutSelectLowerWorkspace);

    // select_left_workspace
    connect(createNewShotcut("shortcuts.tab.select_left_workspace"), &QShortcut::activated, this, &MainWindow::slotShortcutSelectLeftWorkspace);

    // select_right_workspace
    connect(createNewShotcut("shortcuts.tab.select_right_workspace"), &QShortcut::activated, this, &MainWindow::slotShortcutSelectRightWorkspace);

    // close_workspace
    connect(createNewShotcut("shortcuts.tab.close_workspace"), &QShortcut::activated, this, &MainWindow::slotShortcutCloseWorkspace);

    // close_other_workspaces
    connect(createNewShotcut("shortcuts.tab.close_other_workspaces"), &QShortcut::activated, this, &MainWindow::slotShortcutCloseOtherWorkspaces);

    // copy
    connect(createNewShotcut("shortcuts.terminal.copy"), &QShortcut::activated, this, &MainWindow::slotShortcutCopy);

    // paste
    connect(createNewShotcut("shortcuts.terminal.paste"), &QShortcut::activated, this, &MainWindow::slotShortcutPaste);

    // search
    connect(createNewShotcut("shortcuts.terminal.find"), &QShortcut::activated, this, &MainWindow::slotShortcutFind);

    // zoom_in
    connect(createNewShotcut("shortcuts.terminal.zoom_in"), &QShortcut::activated, this, &MainWindow::slotShortcutZoomIn);

    // zoom_out
    connect(createNewShotcut("shortcuts.terminal.zoom_out"), &QShortcut::activated, this, &MainWindow::slotShortcutZoomOut);

    // default_size
    connect(createNewShotcut("shortcuts.terminal.default_size"), &QShortcut::activated, this, &MainWindow::slotShortcutDefaultSize);

    // select_all
    connect(createNewShotcut("shortcuts.terminal.select_all"), &QShortcut::activated, this, &MainWindow::slotShortcutSelectAll);

    // switch_fullscreen
    connect(createNewShotcut("shortcuts.advanced.switch_fullscreen"), &QShortcut::activated, this, &MainWindow::slotShortcutSwitchFullScreen);

    // rename_tab
    connect(createNewShotcut("shortcuts.advanced.rename_title"), &QShortcut::activated, this, &MainWindow::slotShortcutRenameTitle);

    // display_shortcuts
    connect(createNewShotcut("shortcuts.advanced.display_shortcuts"), &QShortcut::activated, this, &MainWindow::slotShortcutDisplayShortcuts);

    // custom_command
    connect(createNewShotcut("shortcuts.advanced.custom_command"), &QShortcut::activated, this, &MainWindow::slotShortcutCustomCommand);

    // remote_management
    connect(createNewShotcut("shortcuts.advanced.remote_management"), &QShortcut::activated, this, &MainWindow::slotShortcutRemoteManage);
    /********************* Modify by n014361 wangpeili End ************************/

    /******** Modify by ut000439 wangpeili 2020-07-17: Super+Tab快捷键   ****************/
    QShortcut *shortcutFoucusOut = new QShortcut(QKeySequence(QKEYSEQUENCE_FOCUSOUT_TIMINAL), this);
    connect(shortcutFoucusOut, &QShortcut::activated, this, &MainWindow::slotShortcutFocusOut);
    /********************* Modify by n014361 wangpeili End ************************/
    /******** Modify by ut000439 wangpeili 2020-07-27: bug 39494   ****************/
    QShortcut *shortcutBuiltinPaste = new QShortcut(QKeySequence(QKEYSEQUENCE_PASTE_BUILTIN), this);
    connect(shortcutBuiltinPaste, &QShortcut::activated, this, &MainWindow::slotShortcutBuiltinPaste);

    /******** Modify by ut001000 renfeixiang 2020-08-28:修改 bug 44477***************/
    QShortcut *shortcutBuiltinCopy = new QShortcut(QKeySequence(QKEYSEQUENCE_COPY_BUILTIN), this);
    connect(shortcutBuiltinCopy, &QShortcut::activated, this, &MainWindow::slotShortcutBuiltinCopy);
    /********************* Modify by n014361 wangpeili End ************************/

    for (int i = 1 ; i <= 9; i++) {
        QString strSwitchLabel = QString("shortcuts.tab.switch_label_win_%1").arg(i);
        QShortcut *switchShortcut = createNewShotcut(strSwitchLabel);
        switchShortcut->setProperty("index", QVariant(i));
        connect(switchShortcut, &QShortcut::activated, this, &MainWindow::slotShortcutSwitchActivated);

    }
}

inline void MainWindow::slotShortcutSwitchActivated()
{
    QShortcut *switchShortcut = qobject_cast<QShortcut *>(sender());
    int i = switchShortcut->property("index").toInt();
    TermWidgetPage *page = currentPage();
    if (page) {
        assert(m_tabbar);
        if ((9 == i) && (m_tabbar->count() > 9)) {
            m_tabbar->setCurrentIndex(m_tabbar->count() - 1);
            return;
        }

        if (i - 1 >= m_tabbar->count()) {
            qInfo() << "i - 1 > tabcount" << i - 1 << m_tabbar->count() << endl;
            return;
        }

        qInfo() << "index" << i - 1 << endl;
        m_tabbar->setCurrentIndex(i - 1);
        return;
    }
    qInfo() << "currentPage nullptr ??";
}

inline void MainWindow::slotShortcutNewTab()
{
    this->addTab(currentPage()->createCurrentTerminalProperties(), true);
}

inline void MainWindow::slotShortcutCloseTab()
{
    TermWidgetPage *page = currentPage();
    if (page)
        closeTab(page->identifier());
}

inline void MainWindow::slotShortcutCloseOtherTabs()
{
    TermWidgetPage *page = currentPage();
    if (page)
        closeOtherTab(page->identifier());
}

inline void MainWindow::slotShortcutPreviousTab()
{
    TermWidgetPage *page = currentPage();
    if (page) {
        int index = m_tabbar->currentIndex();
        index -= 1;
        if (index < 0)
            index = m_tabbar->count() - 1;

        m_tabbar->setCurrentIndex(index);
    }
}

inline void MainWindow::slotShortcutNextTab()
{
    TermWidgetPage *page = currentPage();
    if (page) {
        int index = m_tabbar->currentIndex();
        index += 1;
        if (index == m_tabbar->count())
            index = 0;

        m_tabbar->setCurrentIndex(index);
    }
}

inline void MainWindow::slotShortcutHorizonzalSplit()
{
    // 判读数量是否允许分屏
    if (Service::instance()->isCountEnable()) {
        TermWidgetPage *page = currentPage();
        if (page) {
            if (page->currentTerminal()) {
                int layer = page->currentTerminal()->getTermLayer();
                DSplitter *splitter = qobject_cast<DSplitter *>(page->currentTerminal()->parentWidget());
                if (1 == layer  || (2 == layer  &&  splitter && Qt::Horizontal == splitter->orientation())) {
                    page->split(Qt::Horizontal);
                    return ;
                }
            }
        }
    }
    qInfo() << "can't split vertical  again";
}

inline void MainWindow::slotShortcutVerticalSplit()
{
    // 判读数量是否允许分屏
    if (Service::instance()->isCountEnable()) {
        TermWidgetPage *page = currentPage();
        if (page) {
            if (page->currentTerminal()) {
                int layer = page->currentTerminal()->getTermLayer();
                DSplitter *splitter = qobject_cast<DSplitter *>(page->currentTerminal()->parentWidget());
                if (1 == layer  || (2 == layer  &&  splitter && Qt::Vertical == splitter->orientation())) {
                    page->split(Qt::Vertical);
                    return ;
                }
            }
        }
    }
    qInfo() << "can't split vertical  again";
}

inline void MainWindow::slotShortcutSelectUpperWorkspace()
{
    qInfo() << "Alt+k";
    TermWidgetPage *page = currentPage();
    if (page)
        page->focusNavigation(Qt::TopEdge);
}

inline void MainWindow::slotShortcutSelectLowerWorkspace()
{
    TermWidgetPage *page = currentPage();
    if (page)
        page->focusNavigation(Qt::BottomEdge);
}

inline void MainWindow::slotShortcutSelectLeftWorkspace()
{
    TermWidgetPage *page = currentPage();
    if (page)
        page->focusNavigation(Qt::LeftEdge);
}

inline void MainWindow::slotShortcutSelectRightWorkspace()
{
    TermWidgetPage *page = currentPage();
    if (page)
        page->focusNavigation(Qt::RightEdge);
}

inline void MainWindow::slotShortcutCloseWorkspace()
{
    TermWidgetPage *page = currentPage();
    if (page) {
        qInfo() << "CloseWorkspace";
        page->closeSplit(page->currentTerminal());
    }
}

inline void MainWindow::slotShortcutCloseOtherWorkspaces()
{
    TermWidgetPage *page = currentPage();
    if (page)
        page->closeOtherTerminal();
}

inline void MainWindow::slotShortcutCopy()
{
    TermWidgetPage *page = currentPage();
    if (page)
        page->copyClipboard();
}

inline void MainWindow::slotShortcutPaste()
{
    TermWidgetPage *page = currentPage();
    if (page)
        page->pasteClipboard();
}

inline void MainWindow::slotShortcutFind()
{
    showPlugin(PLUGIN_TYPE_SEARCHBAR);
}

inline void MainWindow::slotShortcutZoomIn()
{
    TermWidgetPage *page = currentPage();
    if (page)
        page->zoomInCurrentTierminal();

    /******** Add by nt001000 renfeixiang 2020-05-20:增加 雷神窗口根据字体大小设置最小高度函数 Begin***************/
    setWindowMinHeightForFont();
    /******** Add by ut001000 renfeixiang 2020-08-07:zoom_in时改变大小，bug#41436***************/
    updateMinHeight();
    /******** Add by nt001000 renfeixiang 2020-05-20:增加 雷神窗口根据字体大小设置最小高度函数 End***************/
}

inline void MainWindow::slotShortcutZoomOut()
{
    TermWidgetPage *page = currentPage();
    if (page)
        page->zoomOutCurrentTerminal();

    /******** Add by nt001000 renfeixiang 2020-05-20:增加 雷神窗口根据字体大小设置最小高度函数 Begin***************/
    setWindowMinHeightForFont();
    /******** Add by ut001000 renfeixiang 2020-08-07:zoom_out时改变大小，bug#41436***************/
    updateMinHeight();
    /******** Add by nt001000 renfeixiang 2020-05-20:增加 雷神窗口根据字体大小设置最小高度函数 End***************/
}

inline void MainWindow::slotShortcutDefaultSize()
{
    TermWidgetPage *page = currentPage();
    if (page)
        page->setFontSize(Settings::instance()->fontSize());

    /******** Add by nt001000 renfeixiang 2020-05-20:增加 雷神窗口根据字体大小设置最小高度函数 Begin***************/
    setWindowMinHeightForFont();
    /******** Add by ut001000 renfeixiang 2020-08-07:default_size时改变大小，bug#41436***************/
    updateMinHeight();
    /******** Add by nt001000 renfeixiang 2020-05-20:增加 雷神窗口根据字体大小设置最小高度函数 End***************/
}

inline void MainWindow::slotShortcutSelectAll()
{
    TermWidgetPage *page = currentPage();
    if (page) {
        qInfo() << "selectAll";
        page->selectAll();
    }
}

inline void MainWindow::slotShortcutSwitchFullScreen()
{
    switchFullscreen();
}

inline void MainWindow::slotShortcutRenameTitle()
{
    showPlugin(PLUGIN_TYPE_NONE);
    TermWidgetPage *page = currentPage();
    if (page)
        page->showRenameTitleDialog();
}

inline void MainWindow::slotShortcutDisplayShortcuts()
{
    displayShortcuts();
}

inline void MainWindow::slotShortcutCustomCommand()
{
    if (PLUGIN_TYPE_CUSTOMCOMMAND == m_CurrentShowPlugin)
        showPlugin(PLUGIN_TYPE_NONE);
    else
        showPlugin(PLUGIN_TYPE_CUSTOMCOMMAND);
}

inline void MainWindow::slotShortcutRemoteManage()
{
    if (PLUGIN_TYPE_REMOTEMANAGEMENT == m_CurrentShowPlugin)
        showPlugin(PLUGIN_TYPE_NONE);
    else
        showPlugin(PLUGIN_TYPE_REMOTEMANAGEMENT);
}

inline void MainWindow::slotShortcutFocusOut()
{
    qInfo() << "focusout timinal is activated!" << QKEYSEQUENCE_FOCUSOUT_TIMINAL;
    DIconButton *addButton = m_tabbar->findChild<DIconButton *>("AddButton");
    if (addButton != nullptr)
        addButton->setFocus();
    else
        qInfo() << "can not found AddButton in DIconButton";
}

inline void MainWindow::slotShortcutBuiltinPaste()
{
    qInfo() << "built in paste shortcut is activated!" << QKEYSEQUENCE_PASTE_BUILTIN;
    TermWidgetPage *page = currentPage();
    if (page)
        page->pasteClipboard();
}

inline void MainWindow::slotShortcutBuiltinCopy()
{
    qInfo() << "built in copy shortcut is activated!" << QKEYSEQUENCE_COPY_BUILTIN;
    TermWidgetPage *page = currentPage();
    if (page)
        page->copyClipboard();
}

void MainWindow::initConnections()
{
    connect(this, &MainWindow::mainwindowClosed, WindowsManager::instance(), &WindowsManager::onMainwindowClosed);
    connect(Settings::instance(), &Settings::terminalSettingChanged, this, &MainWindow::onTerminalSettingChanged);
    connect(Settings::instance(), &Settings::windowSettingChanged, this, &MainWindow::onWindowSettingChanged);
    connect(Settings::instance(), &Settings::shortcutSettingChanged, this, &MainWindow::onShortcutSettingChanged);
    connect(this, &MainWindow::newWindowRequest, this, &MainWindow::onCreateNewWindow);
    connect(qApp, &QGuiApplication::applicationStateChanged, this, &MainWindow::onApplicationStateChanged);
}

void MainWindow::showPlugin(const QString &name)
{
    bool bSetFocus = false;
    // 当焦点不在终端时，调用插件，并直接进入焦点
    if (qApp->focusWidget() != nullptr) {
        if (QString(qApp->focusWidget()->metaObject()->className()) != TERM_WIDGET_NAME)
            bSetFocus = true;
    }

    if (m_CurrentShowPlugin.isEmpty())
        return;

    if ((name == m_CurrentShowPlugin) && (PLUGIN_TYPE_NONE == m_CurrentShowPlugin)) {
        // 目前没有列表显示，直接返回
        qInfo() << "no plugin show!";
        return;
    }

    m_CurrentShowPlugin = name;
    if (name != PLUGIN_TYPE_NONE)
        qInfo() << "show Plugin" << name << bSetFocus;

    emit showPluginChanged(name, bSetFocus);
}

void MainWindow::hidePlugin()
{
    if (PLUGIN_TYPE_NONE == m_CurrentShowPlugin)
        return;

    qInfo() << "hide Plugin" << m_CurrentShowPlugin;
    m_CurrentShowPlugin = PLUGIN_TYPE_NONE;
    emit quakeHidePlugin();
}

QString MainWindow::selectedText()
{
    TermWidgetPage *page = currentPage();
    if (page) {
        if (page->currentTerminal())
            return page->currentTerminal()->selectedText();
    }
    qInfo() << "not point terminal??";
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
    if (QEvent::KeyPress == event->type() && currentActivatedTerminal()) {
        // 当前的终端进行操作
        TermWidget *term = currentActivatedTerminal();
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        // 键盘事件 按下回车
        if (Qt::Key_Return == keyEvent->key()  || Qt::Key_Enter == keyEvent->key()) {
            // 判断是否进行下载输入完要下载的文件后按下回车且当前窗口是激活窗口
            if (term->enterSzCommand() && term->isActiveWindow()) {
                //--added by qinyaning(nyq) to slove Unable to download file from server, time: 2020.4.13 18:21--//
                // 先用sz获取输入的文件
                pressEnterKey("\nsz \"${files[@]}\"");
                //-------------------------------------
                // 执行下载操作
                executeDownloadFile();
                // 下载完成,将是否进行下载的标志位设置为false
                term->setEnterSzCommand(false);
            }
        }
        if ((Qt::ControlModifier == keyEvent->modifiers()) && (Qt::Key_C == keyEvent->key()  ||  Qt::Key_D == keyEvent->key())) {
            // 当点击ctrl+c或者ctrl+d时判断,当前窗口是否正在等待下载输入
            if (term->enterSzCommand()) {
                // 若是则将是否进行下载的标志位设置为false
                term->setEnterSzCommand(false);
            }
        }
        /******** Modify by ut000610 daizhengwen 2020-09-07:焦点在推出全屏上,点击Enter或Space Begin***************/
        // fix#bug 46680
        if ((QStringLiteral("Dtk::Widget::DWindowQuitFullButton") == watched->metaObject()->className())
                && (Qt::Key_Return == keyEvent->key()  ||  Qt::Key_Enter == keyEvent->key()  ||  Qt::Key_Space == keyEvent->key())) {
            // 退出全屏
            switchFullscreen(false);
            return true;
        }
        /********************* Modify by ut000610 daizhengwen End ************************/
        if ((Qt::ControlModifier == keyEvent->modifiers()) && (Qt::Key_S == keyEvent->key())) {

            assert(term);
            if (term->isActiveWindow() && term->flowControlEnabled())
                term->showFlowMessage(true);
        }

        if ((Qt::ControlModifier == keyEvent->modifiers()) && (Qt::Key_Q == keyEvent->key())) {
            assert(term);
            if (term->isActiveWindow() && term->flowControlEnabled())
                term->showFlowMessage(false);
        }
    }

    //if (watched == this) {
    if (QEvent::KeyPress == event->type()) {
        //将事件转化为键盘事件
        QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
        // 全局按下ESC键返回终端，过滤掉个别情况
        if (Qt::Key_Escape == key_event->key()) {
            QString filterReason; // 过滤原因
            do {
                // 不干扰其它对话框使用ESC（一般框会遮盖enable false)
                if (!this->isEnabled()) {
                    filterReason = "mainwidow is disable";
                    break;
                }
                // 未激活不生效
                if (!this->isActiveWindow()) {
                    filterReason = "mainwidow is not active";
                    break;
                }
                // 不干扰终端使用ESC
                if (TERM_WIDGET_NAME == QString(qApp->focusWidget()->metaObject()->className())) {
                    filterReason = "focusWidget is terminnal";
                    /***add by ut001121 zhangmeng 20200922 修复BUG48960***/
                    showPlugin(PLUGIN_TYPE_NONE);
                    break;
                }
                // 如果有菜单出现的时候，ESC无效
                for (QWidget *top : qApp->topLevelWidgets()) {
                    if ("QMenu" == QString(top->metaObject()->className()) && top->isVisible()) {
                        filterReason = QString(top->metaObject()->className()) + " is display";
                        break;
                    }
                }
            } while (false);

            if (!filterReason.isEmpty()) {
                qInfo() << "Esc is not effect, reason:" << filterReason;
            } else {
                focusCurrentPage();
                showPlugin(PLUGIN_TYPE_NONE);
                return  true;
            }
        }
    }

    return DMainWindow::eventFilter(watched, event);
}

void MainWindow::onTerminalSettingChanged(const QString &keyName)
{
    if (QStringLiteral("basic.interface.blurred_background") == keyName) {
        setEnableBlurWindow(Settings::instance()->backgroundBlur());
        return;
    }
}

void MainWindow::onWindowSettingChanged(const QString &keyName)
{
    // use_on_starting重启生效
    if (keyName == QStringLiteral("advanced.window.use_on_starting")) {
        QString state = Settings::instance()->settings->option("advanced.window.use_on_starting")->value().toString();
        if ("window_normal" == state) {
            m_IfUseLastSize = true;
            /******** Modify by nt001000 renfeixiang 2020-05-25: 文件wininfo-config.conf中参数,使用定义更换window_width，window_height Begin***************/
            m_winInfoConfig->setValue(CONFIG_WINDOW_WIDTH, WINDOW_DEFAULT_WIDTH);
            m_winInfoConfig->setValue(CONFIG_WINDOW_HEIGHT, WINDOW_DEFAULT_HEIGHT);
            /******** Modify by nt001000 renfeixiang 2020-05-25: 文件wininfo-config.conf中参数,使用定义更换window_width，window_height End***************/
            qInfo() << "change value change value change value";
        } else {
            m_IfUseLastSize = false;
        }
        qInfo() << "settingValue[" << keyName << "] changed to " << state
                << ", auto effective when next start!";
        return;
    }
    // auto_hide_raytheon_window在使用中自动读取生效
    if ((QStringLiteral("advanced.window.auto_hide_raytheon_window") == keyName) || (QStringLiteral("advanced.window.use_on_starting") == keyName)) {
        qInfo() << "settingValue[" << keyName << "] changed to " << Settings::instance()->OutputtingScroll()
                << ", auto effective when happen";
        /***mod begin by ut001121 zhangmeng 20200528 修复BUG28920***/
        onAppFocusChangeForQuake();
        /***mod end by ut001121***/
        return;
    }

    qInfo() << "settingValue[" << keyName << "] changed is not effective";
}

void MainWindow::onShortcutSettingChanged(const QString &keyName)
{
    qInfo() << "Shortcut[" << keyName << "] changed";
    if (m_builtInShortcut.contains(keyName)) {
        QString value = Settings::instance()->settings->option(keyName)->value().toString();
        //m_builtInShortcut[keyName]->setKey(QKeySequence(value));
        // 设置中快捷键变化,重新注册快捷键,使用小写 up2down
        m_builtInShortcut[keyName]->setKey(Utils::converUpToDown(value));
        return;
    }

    qInfo() << "Shortcut[" << keyName << "] changed is unknown!";
}

void MainWindow::setNewTermPage(TermWidgetPage *termPage, bool activePage)
{
    if (nullptr == termPage) {
        qInfo() << "termPage is nullptr!";
        return;
    }

    m_termWidgetPageMap.insert(termPage->identifier(), termPage);

    m_termStackWidget->addWidget(termPage);
    if (activePage)
        m_termStackWidget->setCurrentWidget(termPage);
}

void MainWindow::createNewTab()
{
    addTab(currentPage()->createCurrentTerminalProperties(), true);
}

void MainWindow::displayShortcuts()
{
    QPoint pos = calculateShortcutsPreviewPoint();

    QJsonArray jsonGroups;
    createJsonGroup("terminal", jsonGroups);
    createJsonGroup("tab", jsonGroups);
    createJsonGroup("advanced", jsonGroups);
    QJsonObject shortcutObj;
    shortcutObj.insert("shortcut", jsonGroups);

    QJsonDocument doc(shortcutObj);

    QStringList shortcutString;
    QByteArray array = doc.toJson();
    QString param1 = "-j=" + QString(array.data());
    QString param2 = "-p=" + QString::number(pos.x()) + "," + QString::number(pos.y());
    shortcutString << param1 << param2;

    QScopedPointer<QProcess> shortcutViewProcess(new QProcess());
    shortcutViewProcess->startDetached("deepin-shortcut-viewer", shortcutString);
}

void MainWindow::createJsonGroup(const QString &keyCategory, QJsonArray &jsonGroups)
{
    qInfo() << keyCategory;

    QString strGroupName = "";
    if ("tab" == keyCategory)
        strGroupName =  QObject::tr("Tabs");
    else if ("terminal" == keyCategory)
        strGroupName =  QObject::tr("Terminal");
    else if ("advanced" == keyCategory)
        strGroupName =  QObject::tr("Others");
    else
        return;

    QString groupname = "shortcuts." + keyCategory;

    QJsonArray JsonArry;
    for (auto &opt :
            Settings::instance()->settings->group(groupname)->options()) {  // Settings::instance()->settings->keys())
        QJsonObject jsonItem;
        QString name = QObject::tr(opt->name().toUtf8().data());
        jsonItem.insert("name", name);
        jsonItem.insert("value", opt->value().toString());
        JsonArry.append(jsonItem);
    }

    QString swithFocusToIcon = QObject::tr("Switch focus to \"+\" icon");
    if ("advanced" == keyCategory) {
        QJsonObject jsonItem;
        jsonItem.insert("name", swithFocusToIcon);
        jsonItem.insert("value", "Super+Tab");
        JsonArry.append(jsonItem);
    }

    /************************ Add by sunchengxi 2020-06-08:json重新排序，快捷键显示顺序调整 Begin************************/
    //default-config.json 文件增加的跟此处相关字段，此处相应添加，保证显示。
    //用三个条件分开，清晰，方便后续调整扩展维护，代码稍微多点。
    if ("terminal" == keyCategory) {
        QStringList strList;
        strList << QObject::tr("Copy")  << QObject::tr("Paste") << QObject::tr("Find") << QObject::tr("Zoom in") << QObject::tr("Zoom out") << QObject::tr("Default size") << QObject::tr("Select all");
        QJsonArray newJsonArry;
        for (int i = 0; i < strList.size(); i++) {
            for (int j = 0; j < JsonArry.size(); j++) {
                QJsonObject jsonItem = JsonArry[j].toObject();
                if (jsonItem.value("name") == strList[i]) {
                    newJsonArry.append(jsonItem);
                    break;
                }
            }
        }
        JsonArry = newJsonArry;
    }
    if ("tab" == keyCategory) {
        QStringList strList;
        strList << QObject::tr("New tab") << QObject::tr("Close tab") << QObject::tr("Close other tabs") << QObject::tr("Previous tab") << QObject::tr("Next tab")
                << QObject::tr("Select tab") << QObject::tr("Vertical split") << QObject::tr("Horizontal split") << QObject::tr("Select upper workspace") << QObject::tr("Select lower workspace")
                << QObject::tr("Select left workspace") << QObject::tr("Select right workspace") << QObject::tr("Close workspace") << QObject::tr("Close other workspaces")
                << QObject::tr("Go to tab 1") << QObject::tr("Go to tab 2") << QObject::tr("Go to tab 3")
                << QObject::tr("Go to tab 4") << QObject::tr("Go to tab 5") << QObject::tr("Go to tab 6")
                << QObject::tr("Go to tab 7") << QObject::tr("Go to tab 8") << QObject::tr("Go to tab 9");
        QJsonArray newJsonArry;
        for (int i = 0; i < strList.size(); i++) {
            for (int j = 0; j < JsonArry.size(); j++) {
                QJsonObject jsonItem = JsonArry[j].toObject();
                if (jsonItem.value("name") == strList[i]) {
                    newJsonArry.append(jsonItem);
                    break;
                }
            }
        }
        JsonArry = newJsonArry;
    }
    if ("advanced" == keyCategory) {
        QStringList strList;
        strList << QObject::tr("Fullscreen") << QObject::tr("Rename title") << QObject::tr("Display shortcuts") << QObject::tr("Custom commands") << QObject::tr("Remote management")
                << swithFocusToIcon;
        QJsonArray newJsonArry;
        for (int i = 0; i < strList.size(); i++) {
            for (int j = 0; j < JsonArry.size(); j++) {
                QJsonObject jsonItem = JsonArry[j].toObject();
                if (jsonItem.value("name") == strList[i]) {
                    newJsonArry.append(jsonItem);
                    break;
                }
            }
        }
        JsonArry = newJsonArry;
    }
    /************************ Add by sunchengxi 2020-06-08:json重新排序，快捷键显示顺序调整   End************************/

    QJsonObject JsonGroup;
    JsonGroup.insert("groupName", strGroupName);
    JsonGroup.insert("groupItems", JsonArry);
    jsonGroups.append(JsonGroup);
}

QShortcut *MainWindow::createNewShotcut(const QString &key, bool AutoRepeat)
{
    QString value = Settings::instance()->settings->option(key)->value().toString();
    //bug#89372，"显示快捷键"选项，在设置界面隐藏，仅默认值生效
    if ("shortcuts.advanced.display_shortcuts" == key)
        value = "Ctrl+Shift+?";
    // 初始化设置中的快捷键,使用小写 up2down dzw 20201215
    QShortcut *shortcut = new QShortcut(Utils::converUpToDown(value), this);
    m_builtInShortcut[key] = shortcut;
    shortcut->setAutoRepeat(AutoRepeat);
    return shortcut;
}


void MainWindow::remoteUploadFile()
{
    //下载弹窗加载
    QString curPath = QDir::currentPath();
    QString dlgTitle = QObject::tr("Select file to upload");

    // 设置弹窗
    DFileDialog *dialog = new DFileDialog(this, dlgTitle, curPath);
    connect(dialog, &DFileDialog::finished, this, &MainWindow::onUploadFileDialogFinished);
    dialog->setAcceptMode(QFileDialog::AcceptOpen);
    dialog->setFileMode(QFileDialog::ExistingFiles);
    dialog->setLabelText(QFileDialog::Accept, QObject::tr("Upload"));
    dialog->open();
}

inline void MainWindow::onUploadFileDialogFinished(int code)
{
    DFileDialog *dialog = qobject_cast<DFileDialog *>(sender());
    if (nullptr == dialog)
        return;

    QStringList fileName;

    if (QDialog::Accepted == code)
        fileName = dialog->selectedFiles();
    else
        // 选择文件，却点击了叉号
        fileName = QStringList();

    if (!fileName.isEmpty()) {
        QString strTxt = "sz ";
        for (QString &str : fileName) {
            strTxt += str;
            strTxt += " ";
        }
        remoteUploadFile(strTxt);
    } else {
        qInfo() << "remoteUploadFile file name is Null";
    }
}

void MainWindow::slotDialogSelectFinished(int code)
{
    DFileDialog *dialog = qobject_cast<DFileDialog *>(sender());
    if (QDialog::Accepted == code  && !dialog->selectedFiles().isEmpty()) {
        QStringList list = dialog->selectedFiles();
        const QString dirName = list.first();
        downloadFilePath = dirName;
    } else {
        downloadFilePath = "";
    }

    TermWidget *term = currentActivatedTerminal();

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

/**
 * Download file from remote server
 */

void MainWindow::remoteDownloadFile()
{
    QString curPath = QDir::currentPath();
    QString dlgTitle = QObject::tr("Select a directory to save the file");

    DFileDialog *dialog = new DFileDialog(this, dlgTitle, curPath);
    dialog->setAcceptMode(QFileDialog::AcceptOpen);
    dialog->setFileMode(QFileDialog::Directory);
    dialog->setOption(DFileDialog::DontConfirmOverwrite);
    dialog->setLabelText(QFileDialog::Accept, QObject::tr("Select"));
    connect(dialog, &DFileDialog::finished, this, &MainWindow::slotDialogSelectFinished);
    dialog->open();
}

void MainWindow::onApplicationStateChanged(Qt::ApplicationState state)
{
    Q_UNUSED(state)
    return;
}

inline void MainWindow::slotCustomCommandActionTriggered()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!this->isActiveWindow())
        return;

    QString command = action->data().toString();
    if (!command.endsWith('\n'))
        command.append('\n');

    currentPage()->sendTextToCurrentTerm(command);
}

void MainWindow::addCustomCommandSlot(QAction *newAction)
{
    QAction *action = newAction;
    // 注册自定义快捷键,使用小写 up2down dzw 20201215
    action->setShortcut(Utils::converUpToDown(action->shortcut()));
    addAction(action);

    connect(action, &QAction::triggered, this, &MainWindow::slotCustomCommandActionTriggered);

}

void MainWindow::removeCustomCommandSlot(QAction *newAction)
{
    removeAction(newAction);
}

void MainWindow::OnHandleCloseType(int result, Utils::CloseType type)
{
    // 弹窗隐藏或消失
    Service::instance()->setIsDialogShow(this, false);
    if (result != 1) {
        qInfo() << "user cancle close";
        return;
    }

    TermWidgetPage *page = currentPage();
    if (nullptr == page) {
        qInfo() << "null pointer of currentPage ???";
        return;
    }

    //以下所有接口二次重入
    switch (type) {
    case Utils::CloseType_Window:
        m_hasConfirmedClose = true;
        close();
        break;
    case Utils::CloseType_Tab:
        closeTab(page->identifier(), true);
        break;
    case Utils::CloseType_OtherTab:
        closeOtherTab(page->identifier(), true);
        break;
    case Utils::CloseType_Terminal://Terminal相关的关闭弹框操作
        page->closeSplit(page->currentTerminal(), true);
        break;
    case Utils::CloseType_OtherTerminals:
        page->closeOtherTerminal(true);
        break;
    default:
        break;
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    Q_UNUSED(event);
}

void MainWindow::executeDownloadFile()
{
    //--modified by qinyaning(nyq) to slove Unable to download file from server, time: 2020.4.13 18:21--//
    // 前一条命令执行
    currentPage()->sendTextToCurrentTerm("\r\n");
    // 执行玩等待1s直到文件传输结束
    sleep(1000);
    // ctrl+@ 进入当前机器
    pressCtrlAt();
    // cd 到当前机器的制定文件夹下
    sleep(100);
    QString strCd = "cd " + downloadFilePath;
    currentPage()->sendTextToCurrentTerm(strCd);
    // 将文件下载下来
    QString strRz = "\r\nrz -be";
    // 执行下载命令
    currentPage()->sendTextToCurrentTerm(strRz);
    // 将下载路径置空
    downloadFilePath = "";
    //-------------------------------------------
}

void MainWindow::pressCtrlAt()
{
    QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_At, Qt::ControlModifier);
    if (focusWidget())
        QApplication::sendEvent(focusWidget(), &keyPress);
}

void MainWindow::pressCtrlU()
{
    QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_U, Qt::ControlModifier);
    if (focusWidget())
        QApplication::sendEvent(focusWidget(), &keyPress);
}

void MainWindow::sleep(int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while (QTime::currentTime() < dieTime) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
}

//--added by qinyaning(nyq) to slove Unable to download file from server, time: 2020.4.13 18:21--//

void MainWindow::pressEnterKey(const QString &text)
{
    QKeyEvent event(QEvent::KeyPress, 0, Qt::NoModifier, text);
    if (focusWidget())
        QApplication::sendEvent(focusWidget(), &event);  // expose as a big fat keypress event
}

void MainWindow::createWindowComplete()
{
    m_WindowCompleteTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
}

void MainWindow::firstTerminalComplete()
{
    m_FirstTerminalCompleteTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    qInfo() << "app create all complete," << "MainWindowID = " << m_MainWindowID << ",all time use" << m_FirstTerminalCompleteTime - m_ReferedAppStartTime << "ms";
    qInfo() << "before entry use" << m_CreateWindowTime - m_ReferedAppStartTime << "ms";
    // 创建mainwidow时间，这个时候terminal并没有创建好，不能代表什么。
    qInfo() << "cretae first Terminal use" << m_FirstTerminalCompleteTime - m_CreateWindowTime << "ms";
}

QObjectList MainWindow::getNamedChildren(QObject *obj)
{
    QObjectList list;
    if (nullptr == obj)
        return list;

    foreach (QObject *o, obj->children()) {
        if (!o->objectName().isEmpty()) {
            list << o;
        }
        list << getNamedChildren(o);
    }
    return list;
}

void MainWindow::setTitlebarNoFocus(QWidget *titlebar)
{
    foreach (QObject *obj, getNamedChildren(titlebar)) {
        QWidget *w = qobject_cast<QWidget *>(obj);
        if (w)
            w->setFocusPolicy(Qt::NoFocus);
    }
}

qint64 MainWindow::createNewMainWindowTime()
{
    // 当前时间减去创建时间
    return (QDateTime::currentDateTime().toMSecsSinceEpoch() - m_ReferedAppStartTime);
}

int MainWindow::getDesktopIndex() const
{
    return m_desktopIndex;
}

void MainWindow::checkExtendThemeItem(const QString &expandThemeStr, QAction *&action)
{

    if (Settings::instance()->m_configCustomThemePath == expandThemeStr) {
        // 判断是否是自定义
        action = themeCustomAction;
    }
    if (themeBuiltinActionMap.contains(expandThemeStr)) {
        // 判断是否是内置项
        action = themeBuiltinActionMap[expandThemeStr];
    }
    if (expandThemeStr == THEME_SYSTEN) {
        action = autoThemeAction;
    }

    // 默认深色主题
    if (expandThemeStr.isEmpty()) {
        action = darkThemeAction;
    }
}

void MainWindow::checkThemeItem()
{
    bool disableDtkSwitchThemeMenu = qEnvironmentVariableIsSet("KLU_DISABLE_MENU_THEME");
    //disableDtkSwitchThemeMenu = true; //主题显示开关控制
    //观察dtk中klu不支持主题功能。不显示主题相关部分。
    if (disableDtkSwitchThemeMenu)
        return;

    QAction *action;
    // 从配置取值
    QString  expandThemeStr = Settings::instance()->extendColorScheme();
    // 兼容处理
    if (expandThemeStr.isEmpty()) {
        expandThemeStr = Settings::instance()->colorScheme();
    } else {
        Settings::instance()->setExtendColorScheme(THEME_NO);
    }
    // 根据当前系统主题先选择默认选项
    switch (DGuiApplicationHelper::instance()->paletteType()) {
    case DGuiApplicationHelper::LightType:
        action = lightThemeAction; break;
    case DGuiApplicationHelper::DarkType:
        action = darkThemeAction; break;
    default:
        //默认是跟随系统
        action = autoThemeAction; break;
    }
    // 根据配置校验选项
    checkExtendThemeItem(expandThemeStr, action);

    //勾选主题项 => 初始化
    action->setChecked(true);
    currCheckThemeAction = action;
}

void MainWindow::addThemeMenuItems()
{
    QMenu *menu = m_menu;
    QAction *themeSeparator = nullptr;

    //下面代码参考dtk编写
    bool disableDtkSwitchThemeMenu = qEnvironmentVariableIsSet("KLU_DISABLE_MENU_THEME");
    //disableDtkSwitchThemeMenu = true;//主题显示开关控制
    //观察dtk中klu不支持主题功能。不显示主题相关部分。
    if (!disableDtkSwitchThemeMenu) {
        //浅色 深色 跟随系统  的 翻译 不需要终端自己翻译，这里直接用dtk的翻译即可
        switchThemeMenu = new SwitchThemeMenu(qApp->translate("TitleBarMenu", THEME), menu);
        lightThemeAction = switchThemeMenu->addAction(qApp->translate("TitleBarMenu", THEME_SYSTEN_LIGHT));
        darkThemeAction = switchThemeMenu->addAction(qApp->translate("TitleBarMenu", THEME_SYSTEN_DARK));
        autoThemeAction = switchThemeMenu->addAction(qApp->translate("TitleBarMenu", THEME_SYSTEN));
        switchThemeMenu->addSeparator();

        //添加内置主题项列表
        //设置主题项可选
        autoThemeAction->setCheckable(true);
        autoThemeAction->setData(THEME_SYSTEN);
        lightThemeAction->setCheckable(true);
        lightThemeAction->setData(THEME_LIGHT);
        darkThemeAction->setCheckable(true);
        darkThemeAction->setData(THEME_DARK);

        //创建主题项快捷键组
        group = new QActionGroup(switchThemeMenu);
        group->addAction(autoThemeAction);
        group->addAction(lightThemeAction);

        addThemeFromConfig();

        // 自定义主题
        themeCustomAction = switchThemeMenu->addAction(tr("Custom Theme"));
        themeCustomAction->setData(THEME_CUSTOM);
        group->addAction(darkThemeAction);
        themeCustomAction->setCheckable(true);
        group->addAction(themeCustomAction);

        menu->addMenu(switchThemeMenu);
        themeSeparator = menu->addSeparator();

        switchThemeMenu->menuAction()->setVisible(true);
        themeSeparator->setVisible(true);

        QObject::connect(group, SIGNAL(triggered(QAction *)),
                         this, SLOT(themeActionTriggeredSlot(QAction *)));
        QObject::connect(group, SIGNAL(hovered(QAction *)),
                         this, SLOT(themeActionHoveredSlot(QAction *)));

        checkThemeItem();

        connect(switchThemeMenu, SIGNAL(mainWindowCheckThemeItemSignal()), this, SLOT(themeRecovery()));
        connect(switchThemeMenu, SIGNAL(menuHideSetThemeSignal()), this, SLOT(themeRecovery()));
    }
}

void MainWindow::addThemeFromConfig()
{
    QStringList themeList = QTermWidget::availableColorSchemes();
    themeList.sort();
    int themeCount = themeList.count();
    for (int i = 0; i < themeCount; ++i) {
        QString strTheme = themeList[i];
        if (strTheme == THEME_DARK || strTheme == THEME_LIGHT
                || strTheme == "customTheme") {
            // 将深、浅色和自定义主题主题抛弃
            continue;
        }
        QAction *themeItem = switchThemeMenu->addAction(strTheme);
        themeItem->setCheckable(true);
        group->addAction(themeItem);
        themeBuiltinActionMap.insert(strTheme, themeItem);
    }
}

void MainWindow::switchThemeAction(QAction *action)
{
    bool isCustom = false;
    QString colorthemeName = action->text();
    if (action == themeCustomAction) {
        isCustom = true;
    }
    if (action == lightThemeAction) {
        colorthemeName = THEME_LIGHT;
    }
    if (action == darkThemeAction) {
        colorthemeName = THEME_DARK;
    }
    if (action == autoThemeAction) {
        colorthemeName = THEME_SYSTEN;
    }

    setTheme(isCustom, colorthemeName);
}

void MainWindow::setTheme(bool isCustom, const QString &themeName)
{
    if (isCustom) {
        Service::instance()->showCustomThemeSettingDialog(this);
        return;
    }
    QString strColorTheme = themeName;
    Settings::instance()->themeStr = strColorTheme;
    // 写配置文件
    Settings::instance()->setColorScheme(strColorTheme);
}

void MainWindow::customTheme(const QString &themeNameStr)
{
    if (themeNameStr != Settings::instance()->colorScheme()) {
        // 写配置文件
        Settings::instance()->setColorScheme(themeNameStr);
    } else {
        // 修改了自定义配置
        emit Service::instance()->changeColorTheme(themeNameStr);
    }

}

void MainWindow::themeActionTriggeredSlot(QAction *action)
{
    switchThemeAction(action);
    currCheckThemeAction = action;
}

void MainWindow::themeActionHoveredSlot(QAction *action)
{
    QString strColorTheme = action->text();
    if (!themeBuiltinActionMap.contains(strColorTheme)) {
        if (action->data().toString() == THEME_CUSTOM) {
            // 选到自定义主题
            strColorTheme = Settings::instance()->m_configCustomThemePath;
        } else {
            strColorTheme = action->data().toString();
        }

    }
    if (switchThemeMenu->hoveredThemeStr != strColorTheme) {
        switchThemeMenu->hoveredThemeStr = strColorTheme;
        emit Service::instance()->changeColorTheme(strColorTheme);
    }
}

void MainWindow::themeRecovery()
{
    emit Service::instance()->changeColorTheme(Settings::instance()->colorScheme());
}

void MainWindow::onCommandActionTriggered()
{
    QAction *commandAction = qobject_cast<QAction *>(sender());

    qInfo() << "commandAction->data().toString() is triggered" << this;
    if (!this->isActiveWindow())
        return ;

    QString command = commandAction->data().toString();
    if (!command.endsWith('\n'))
        command.append('\n');

    this->currentPage()->sendTextToCurrentTerm(command);
}

inline void MainWindow::updateWindowTitle()
{
    setWindowTitle(QString("%1 - %2").arg(m_tabbar->tabText(m_tabbar->currentIndex())).arg(QObject::tr("Terminal")));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 普通终端窗口
*/
NormalWindow::NormalWindow(TermProperties properties, QWidget *parent): MainWindow(properties, parent)
{
    Q_ASSERT(m_isQuakeWindow == false);
    setObjectName("NormalWindow");
    initUI();
    initConnections();
    initShortcuts();
    createWindowComplete();
    setIsQuakeWindowTab(false);
}

NormalWindow::~NormalWindow()
{

}

void NormalWindow::initTitleBar()
{
    // titleba在普通模式和雷神模型不一样的功能
    m_titleBar = new TitleBar(this);
    m_titleBar->setObjectName("NormalWindowTitleBar");//Add by ut001000 renfeixiang 2020-08-14
    m_titleBar->setTabBar(m_tabbar);

    titlebar()->setCustomWidget(m_titleBar);
    titlebar()->setTitle("");
    titlebar()->setIcon(QIcon::fromTheme("deepin-terminal"));
    titlebar()->setAutoHideOnFullscreen(true);

    //设置titlebar焦点策略为不抢占焦点策略，防止点击titlebar后终端失去输入焦点
    titlebar()->setFocusPolicy(Qt::NoFocus);
    initOptionButton();
    initOptionMenu();

    /******** Modify by ut000439 wangpeili 2020-07-22:  SP3.1 DTK TAB控件 ****************/
    // 清理titlebar、titlebar所有控件不可获取焦点
    Utils::clearChildrenFocus(titlebar());
    Utils::clearChildrenFocus(m_tabbar);
    /********************* Modify by n014361 wangpeili End ************************/
}

void NormalWindow::initWindowAttribute()
{
    // init window state.
    QString windowState = getConfigWindowState();
    if ("window_maximum" == windowState) {
        setDefaultLocation();
        setWindowState(this->windowState() | Qt::WindowMaximized);
    } else if ("fullscreen" == windowState) {
        setDefaultLocation();
        switchFullscreen(true);
    } else if ("split_screen" == windowState) {
        setWindowRadius(0);
        resize(halfScreenSize());
    } else {
        m_IfUseLastSize = true;
        int saveWidth = m_winInfoConfig->value(CONFIG_WINDOW_WIDTH).toInt();
        int saveHeight = m_winInfoConfig->value(CONFIG_WINDOW_HEIGHT).toInt();
        qInfo() << "load window_width: " << saveWidth;
        qInfo() << "load window_height: " << saveHeight;
        // 如果配置文件没有数据
        if (0 == saveWidth || 0 == saveHeight) {
            saveWidth = WINDOW_DEFAULT_WIDTH;
            saveHeight = WINDOW_DEFAULT_HEIGHT;
        }
        resize(QSize(saveWidth, saveHeight));
        singleFlagMove();
    }
}

void NormalWindow::saveWindowSize()
{
    // 过滤普通模式的特殊窗口
    if (!m_IfUseLastSize)
        return;

    // (真.假)半屏窗口大小时就不记录了
    /******** Modify by ut001000 renfeixiang 2020-07-03:fix# 36482 ***************/
    // 1.高度-1,如果不-1, 半屏启动-》最大化-》后无法正常还原
    // 2.+ QSize(0, 1)，拖动终端到半屏时触发
    if ((size() == halfScreenSize()) || size() == (halfScreenSize() + QSize(0, 1)))
        return;

    //bug#110002：正常窗口，wayland的windowState为WindowActive 而非 WindowNoState，故按下面方法判断窗口的正常状态：非最大、非最小、非全屏
    if (!windowState().testFlag(Qt::WindowMaximized)
            && !windowState().testFlag(Qt::WindowFullScreen)
            && !windowState().testFlag(Qt::WindowMinimized)
       ) {
        /******** Modify by nt001000 renfeixiang 2020-05-25: 文件wininfo-config.conf中参数,使用定义更换window_width，window_height Begin***************/
        // 记录最后一个正常窗口的大小
        m_winInfoConfig->setValue(CONFIG_WINDOW_WIDTH, width());
        m_winInfoConfig->setValue(CONFIG_WINDOW_HEIGHT, height());
        qInfo() << "save windows size:" << width() << height();
        /******** Modify by nt001000 renfeixiang 2020-05-25: 文件wininfo-config.conf中参数,使用定义更换window_width，window_height End***************/
    }
}

void NormalWindow::switchFullscreen(bool forceFullscreen)
{
    if (forceFullscreen || !window()->windowState().testFlag(Qt::WindowFullScreen))
        window()->setWindowState(windowState() | Qt::WindowFullScreen);
    else
        window()->setWindowState(windowState() & ~Qt::WindowFullScreen);

    // 全屏和取消全屏后，都将焦点设置回终端
    focusCurrentPage();
}

QPoint NormalWindow::calculateShortcutsPreviewPoint()
{
    QRect rect = window()->geometry();
    return QPoint(rect.x() + rect.width() / 2, rect.y() + rect.height() / 2);
}

void NormalWindow::onAppFocusChangeForQuake(bool checkIsActiveWindow)
{
    Q_UNUSED(checkIsActiveWindow)
    return;
}

void NormalWindow::changeEvent(QEvent *event)
{
    QMainWindow::changeEvent(event);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 雷神终端窗口
*/
QuakeWindow::QuakeWindow(TermProperties properties, QWidget *parent): MainWindow(properties, parent),
    // 初始化雷神resize的定时器
    m_resizeTimer(new QTimer(this))
{
    Q_ASSERT(m_isQuakeWindow == true);
    setObjectName("QuakeWindow");
    initUI();
    initConnections();
    initShortcuts();
    createWindowComplete();
    setIsQuakeWindowTab(true);
    // 设置雷神resize定时器属性
    m_resizeTimer->setSingleShot(true);
    // 绑定信号槽
    connect(m_resizeTimer, &QTimer::timeout, this, &QuakeWindow::onResizeWindow);

    //设置窗口属性 不可移动
    sendWindowForhibitMove(true);
}

QuakeWindow::~QuakeWindow()
{
    // 析构resize定时器
    if (nullptr != m_resizeTimer) {
        delete m_resizeTimer;
    }
}

void QuakeWindow::initTitleBar()
{
    // titleba在普通模式和雷神模型不一样的功能
    m_titleBar = new TitleBar(this);
    m_titleBar->setObjectName("QuakeWindowTitleBar");//Add by ut001000 renfeixiang 2020-08-14
    m_titleBar->setTabBar(m_tabbar);

    /** add by ut001121 zhangmeng 20200723 for sp3 keyboard interaction */
    //雷神终端设置系统标题栏为禁用状态,使其不获取焦点,不影响键盘交互操作.
    setTitlebarNoFocus(titlebar());
    titlebar()->setFixedHeight(0);
    m_centralLayout->addWidget(m_titleBar);

    /** del by ut001121 zhangmeng 20200723 for sp3 keyboard interaction*/
#if 0
    //设置titlebar焦点策略为不抢占焦点策略
    titlebar()->setFocusPolicy(Qt::NoFocus);

    // titlebar所有控件不可获取焦点
    Utils::clearChildrenFocus(titlebar());
    Utils::clearChildrenFocus(m_tabbar);

    /******** Modify by ut000439 wangpeili 2020-07-22:  SP3.1 DTK TAB控件 ****************/
    DIconButton *addButton = m_tabbar->findChild<DIconButton *>("AddButton");
    if (addButton != nullptr)
        addButton->setFocusPolicy(Qt::TabFocus);
    else
        qInfo() << "can not found AddButton in DIconButton";

    // 雷神下其它控件一律没有焦点
    DIconButton *optionBtn = titlebar()->findChild<DIconButton *>("DTitlebarDWindowOptionButton");
    if (optionBtn != nullptr)
        optionBtn->setFocusPolicy(Qt::NoFocus);
    else
        qInfo() << "can not found DTitlebarDWindowOptionButton in DTitlebar";

    DIconButton *minBtn = titlebar()->findChild<DIconButton *>("DTitlebarDWindowMinButton");
    if (minBtn != nullptr)
        minBtn->setFocusPolicy(Qt::NoFocus);
    else
        qInfo() << "can not found DTitlebarDWindowMinButton in DTitlebar";

    DIconButton *maxBtn = titlebar()->findChild<DIconButton *>("DTitlebarDWindowMaxButton");
    if (maxBtn != nullptr)
        maxBtn->setFocusPolicy(Qt::NoFocus);
    else
        qInfo() << "can not found DTitlebarDWindowMaxButton in DTitlebar";

    DIconButton *closeBtn = titlebar()->findChild<DIconButton *>("DTitlebarDWindowCloseButton");
    if (closeBtn != nullptr)
        closeBtn->setFocusPolicy(Qt::NoFocus);
    else
        qInfo() << "can not found DTitlebarDWindowCloseButton in DTitlebar";

    /*QWidget::setTabOrder(addButton, optionBtn);
    QWidget::setTabOrder(optionBtn, minBtn);
    QWidget::setTabOrder(minBtn, maxBtn);
    QWidget::setTabOrder(maxBtn, closeBtn);*/
    /********************* Modify by n014361 wangpeili End ************************/
#endif
}

void QuakeWindow::slotWorkAreaResized()
{
    qInfo() << "workAreaResized" << QApplication::desktop()->availableGeometry();
    /******** Modify by nt001000 renfeixiang 2020-05-20:修改成只需要设置雷神窗口宽度,根据字体高度设置雷神最小高度 Begin***************/
    setMinimumWidth(QApplication::desktop()->availableGeometry().width());
    setWindowMinHeightForFont();
    /******** Add by ut001000 renfeixiang 2020-08-07:workAreaResized时改变大小，bug#41436***************/
    updateMinHeight();
    /******** Modify by nt001000 renfeixiang 2020-05-20:修改成只需要设置雷神窗口宽度,根据字体高度设置雷神最小高度 End***************/
    move(QApplication::desktop()->availableGeometry().x(), QApplication::desktop()->availableGeometry().y());
    qInfo() << "size" << size();
    setFixedWidth(QApplication::desktop()->availableGeometry().width());
    return ;
}

void QuakeWindow::initWindowAttribute()
{
    /************************ Add by m000743 sunchengxi 2020-04-27:雷神窗口任务栏移动后位置异常问题 Begin************************/
    setWindowRadius(0);
    //QRect deskRect = QApplication::desktop()->availableGeometry();//获取可用桌面大小
    QDesktopWidget *desktopWidget = QApplication::desktop();
    QRect screenRect = desktopWidget->screenGeometry(); //获取设备屏幕大小
    Qt::WindowFlags windowFlags = this->windowFlags();
    setWindowFlags(windowFlags | Qt::WindowStaysOnTopHint/* | Qt::FramelessWindowHint | Qt::BypassWindowManagerHint*/ /*| Qt::Dialog*/);
    //wayland时需要隐藏WindowTitle
    if (Utils::isWayLand()) {
        setWindowFlag(Qt::FramelessWindowHint);
        m_titleBar->setVerResized(true);
    }
    //add a line by ut001121 zhangmeng 2020-04-27雷神窗口禁用移动(修复bug#22975)
    setEnableSystemMove(false);//    setAttribute(Qt::WA_Disabled, true);

    /******** Modify by m000714 daizhengwen 2020-03-26: 窗口高度超过２／３****************/
    setMinimumSize(screenRect.size().width(), 60);
    setMaximumHeight(screenRect.size().height() * 2 / 3);
    /********************* Modify by m000714 daizhengwen End ************************/
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // 计算屏幕宽度，设置雷神终端宽度
    QList<QScreen *> screenList = qApp->screens();
    int w = screenList[0]->geometry().width();
    for (auto it = screenList.constBegin(); it != screenList.constEnd(); ++it) {
        QRect rect = (*it)->geometry();
        if (rect.x() == 0 && rect.y() == 0) {
            w = rect.width();
            break;
        }
    }
    setFixedWidth(w);
    connect(desktopWidget, &QDesktopWidget::workAreaResized, this, &QuakeWindow::slotWorkAreaResized);

    int saveHeight = getQuakeHeight();
    int saveWidth = screenRect.size().width();
    resize(QSize(saveWidth, saveHeight));
    // 记录雷神高度
    m_quakeWindowHeight = saveHeight;
    move(0, 0);
    /************************ Add by m000743 sunchengxi 2020-04-27:雷神窗口任务栏移动后位置异常问题 End  ************************/

    /******** Add by nt001000 renfeixiang 2020-05-20:增加setQuakeWindowMinHeight函数，设置雷神最小高度 Begin***************/
    setWindowMinHeightForFont();
    /******** Add by nt001000 renfeixiang 2020-05-20:增加setQuakeWindowMinHeight函数，设置雷神最小高度 End***************/
}

void QuakeWindow::saveWindowSize()
{
    // 记录最后一个正常窗口的大小
    /******** Modify by nt001000 renfeixiang 2020-05-25: 文件wininfo-config.conf中参数,使用定义更换quake_window_Height Begin***************/
    //Modify by ut001000 renfeixiang 2020-11-16 非雷神动画时，在保存雷神窗口的高度到配置文件
    if (isNotAnimation) {
        m_winInfoConfig->setValue(CONFIG_QUAKE_WINDOW_HEIGHT, height());
        qInfo() << "save quake_window_Height:" << height() << m_desktopMap[m_desktopIndex] << m_desktopIndex;
    }
    /******** Modify by nt001000 renfeixiang 2020-05-25: 文件wininfo-config.conf中参数,使用定义更换quake_window_Height End***************/
}

void QuakeWindow::switchFullscreen(bool forceFullscreen)
{
    Q_UNUSED(forceFullscreen)
    return;
}

QPoint QuakeWindow::calculateShortcutsPreviewPoint()
{
    //--added by qinyaning(nyq) to solve the problem of can't show center--//
    QRect rect = QApplication::desktop()->availableGeometry();
    //---------------------------------------------------------------------//
    return QPoint(rect.x() + rect.width() / 2, rect.y() + rect.height() / 2);
}

void QuakeWindow::onAppFocusChangeForQuake(bool checkIsActiveWindow)
{
    // 开关关闭，不处理
    if (!Settings::instance()->settings->option("advanced.window.auto_hide_raytheon_window")->value().toBool())
        return;

    // 雷神窗口隐藏，不处理
    if (!isVisible())
        return;

    // 雷神的普通对话框,不处理
    if (Service::instance()->getIsDialogShow())
        return;

    // 雷神设置框显示,不处理
    if (Service::instance()->isSettingDialogVisible() && Service::instance()->getSettingOwner() == this)
        return;

    // 处于激活状态,不处理
    if (checkIsActiveWindow && isActiveWindow())
        return;

    hideQuakeWindow();
}

/******** Add by nt001000 renfeixiang 2020-05-20:增加雷神窗口根据字体和字体大小设置最小高度函数 Begin***************/
void QuakeWindow::setWindowMinHeightForFont()
{
    int height = 0;
    //根据内部term的最小高度设置雷神终端的最小高度, (m_MinHeight-50)/2是内部term的最小高度，50是雷神窗口的标题栏高度
    //add by ut001000 renfeixiang 2020-08-07
    height = (m_MinHeight - WIN_TITLE_BAR_HEIGHT) / 2 + 60;
    setMinimumHeight(height);
}

/******** Add by nt001000 renfeixiang 2020-05-20:增加雷神窗口根据字体和字体大小设置最小高度函数 End***************/
void QuakeWindow::updateMinHeight()
{
    qInfo() << "start updateMinHeight";
    bool hasHorizontalSplit = false;
    int count = m_termStackWidget->count();
    for (int i = 0; i < count; i++) {
        TermWidgetPage *tabPage = qobject_cast<TermWidgetPage *>(m_termStackWidget->widget(i));
        if (tabPage->hasHasHorizontalSplit()) {
            hasHorizontalSplit = true;
            break;
        }
    }
    if (hasHorizontalSplit) {
        //Modify by ut001000 renfeixiang 2020-11-16  因为switchEnableResize函数使用setFixedHeight会改变最小高度值，所以将判断条件删除
        setMinimumHeight(m_MinHeight + 10);
    } else {
        //Modify by ut001000 renfeixiang 2020-11-16 将判断条件删除
        setWindowMinHeightForFont();
    }
}

bool QuakeWindow::isShowOnCurrentDesktop()
{
    return m_desktopMap[m_desktopIndex];
}

void QuakeWindow::hideQuakeWindow()
{
    // 隐藏雷神
    // 记录雷神在当前窗口的状态
    m_desktopMap[m_desktopIndex] = false;
    //Add by ut001000 renfeixiang 2020-11-16 添加雷神动画函数, 雷神窗口隐藏已经放在动画效果结束后
    bottomToTopAnimation();
}

void QuakeWindow::onResizeWindow()
{
    resize(width(), m_quakeWindowHeight);
}

void QuakeWindow::topToBottomAnimation()
{
    if (nullptr == currentPage())
        return;

    isNotAnimation = false;
    this->setMinimumHeight(2); // 避免窗管强制取消模糊。
    currentPage()->setMinimumHeight(currentPage()->height());//设置page的最小高度，让动画效果时，page上信息不因为外框的变小而变小

    //动画代码
    QPropertyAnimation *m_heightAni = new QPropertyAnimation(this, "height");
    m_heightAni->setEasingCurve(QEasingCurve::Linear);
    int durationTime = getQuakeAnimationTime();
    m_heightAni->setDuration(durationTime);
    m_heightAni->setStartValue(2);
    m_heightAni->setEndValue(getQuakeHeight());
    m_heightAni->start(QAbstractAnimation::DeleteWhenStopped);

    connect(m_heightAni, &QPropertyAnimation::finished, this, &QuakeWindow::onTopToBottomAnimationFinished);
}

inline void QuakeWindow::onTopToBottomAnimationFinished()
{
    updateMinHeight();//恢复外框的原有最小高度值
    // 保证currentPage有值，不然ut多线程下可能会崩溃
    if (currentPage())
        currentPage()->setMinimumHeight(this->minimumHeight() - tabbarHeight);//恢复page的原有最小高度值 tabbarHeight是tabbar的高度

    isNotAnimation = true;
    /***add by ut001121 zhangmeng 20200606 切换窗口拉伸属性 修复BUG24430***/
    switchEnableResize();
}

void QuakeWindow::bottomToTopAnimation()
{
    if (!isNotAnimation || nullptr == currentPage()) {
        qInfo() << "bottomToTopAnimation no need to execute.";
        return;
    }

    isNotAnimation = false;
    this->setMinimumHeight(0);//设置最小高度为0,让动画效果流畅
    currentPage()->setMinimumHeight(currentPage()->height());//设置page的最小高度，让动画效果时，page上信息不因为外框的变小而变小

    //动画代码
    QPropertyAnimation *m_heightAni = new QPropertyAnimation(this, "height");
    m_heightAni->setEasingCurve(QEasingCurve::Linear);
    int durationTime = getQuakeAnimationTime();
    m_heightAni->setDuration(durationTime);
    m_heightAni->setStartValue(getQuakeHeight());
    m_heightAni->setEndValue(0);
    m_heightAni->start(QAbstractAnimation::DeleteWhenStopped);

    connect(m_heightAni, &QPropertyAnimation::finished, this, &QuakeWindow::onBottomToTopAnimationFinished);
}

inline void QuakeWindow::onBottomToTopAnimationFinished()
{
    this->hide();
    isNotAnimation = true;
}

void QuakeWindow::setHeight(int h)
{
    this->resize(this->width(), h);
}

int QuakeWindow::getQuakeAnimationTime()
{
    QDesktopWidget *desktopWidget = QApplication::desktop();
    QRect screenRect = desktopWidget->screenGeometry(); //获取设备屏幕大小
    //quakeAnimationBaseTime+quakeAnimationHighDistributionTotalTime的时间是雷神窗口最大高度时动画效果时间
    //动画时间计算方法：3quakeAnimationBaseTime加上(quakeAnimationHighDistributionTotalTime乘以当前雷神高度除以雷神最大高度)所得时间，为各个高度时动画时间
    int quakeAnimationBaseTime = Settings::instance()->QuakeDuration() * 2 / 3;
    int quakeAnimationHighDistributionTotalTime = Settings::instance()->QuakeDuration() / 3;
    int durationTime = quakeAnimationBaseTime + 1.5 * quakeAnimationHighDistributionTotalTime * this->getQuakeHeight() / screenRect.height();
    return durationTime;
}

xcb_atom_t QuakeWindow::internAtom(xcb_connection_t *connection, const char *name, bool only_if_exists)
{
    if (!name || *name == 0)
        return  XCB_NONE;

    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(connection, only_if_exists, strlen(name), name);
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(connection, cookie, 0);

    if (!reply)
        return XCB_NONE;

    xcb_atom_t atom = reply->atom;
    free(reply);

    return atom;
}

xcb_atom_t QuakeWindow::internAtom(const char *name, bool only_if_exists)
{
    return internAtom(QX11Info::connection(), name, only_if_exists);
}

/*
 * 雷神窗口不允许移动，但是会响应 窗管的 窗口移动功能，彼此冲突
 * 和 窗管沟通后 定义_DEEPIN_FORHIBIT_MOVE 属性（1：禁止移动,0:可以移动,默认：可以移动）
 * sendWindowForhibitMove(true)，通过调用xcb相关函数，设置_DEEPIN_FORHIBIT_MOVE属性值为1,禁止窗口移动
 */
void QuakeWindow::sendWindowForhibitMove(bool forhibit)
{
    if (!QX11Info::connection()) {
        qWarning() << "QX11Info::connection() is " << QX11Info::connection();
        return ;
    }

    auto reply = internAtom("_DEEPIN_FORHIBIT_MOVE");
    int32_t ldata = forhibit;
    xcb_change_property(QX11Info::connection(), XCB_PROP_MODE_REPLACE, this->winId(),
                        reply, reply, 32, 1, &ldata);
}

void QuakeWindow::changeEvent(QEvent *event)
{
    // 不是激活事件,不处理
    if (QEvent::ActivationChange == event->type()) {
        bool checkIsActiveWindow = true;
        if (qVersion() >= QString("5.15.0")) {
            if (DeActivationChangeEventList.contains(event)) {
                DeActivationChangeEventList.removeOne(event);
                checkIsActiveWindow = false;
            }
        }
        onAppFocusChangeForQuake(checkIsActiveWindow);
    }

    return QMainWindow::changeEvent(event);
}

void QuakeWindow::showEvent(QShowEvent *event)
{
    /***add begin by ut001121 zhangmeng 20200528 重新获取桌面索引 修复BUG29082***/
    m_desktopIndex = DBusManager::callKDECurrentDesktop();
    /***add end by ut001121***/
    // 记录当前桌面的index为显示状态
    m_desktopMap[m_desktopIndex] = true;

    DMainWindow::showEvent(event);
}

bool QuakeWindow::event(QEvent *event)
{
    /***add begin by ut001121 zhangmeng 20200606 切换窗口拉伸属性 修复BUG24430***/
    //Add by ut001000 renfeixiang 2020-11-16 增加雷神动画的标志isNotAnimation， 雷神动画效果时，不进行窗口拉伸属性
    if (QEvent::HoverMove == event->type() && isNotAnimation)
        switchEnableResize();

    /***add end by ut001121***/

    return MainWindow::event(event);
}

bool QuakeWindow::eventFilter(QObject *watched, QEvent *event)
{
    //fix bug: 64490 勾选“丢失焦点后隐藏雷神窗口”，最小化雷神终端后不能再次调出雷神终端
    // 丢失焦点后隐藏雷神窗口开关打开的情况下
    if (watched == this &&
            Settings::instance()->settings->option("advanced.window.auto_hide_raytheon_window")->value().toBool()) {
        //此处通过该操作禁止雷神终端窗口最小化
        if (event->type() == QEvent::WindowStateChange) {
            event->ignore();
            this->activateWindow();
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
            //queue的方式发送事件
            QTimer::singleShot(0, this, [this]() {
                QEvent *event = new QEvent(QEvent::ActivationChange);
                DeActivationChangeEventList << event;
                qApp->postEvent(this, event);
            });
#endif
            return true;
        }
    }

#if 0
    // 由于MainWindow是qApp注册的时间过滤器,所以这里需要判断
    // 只处理雷神的事件 QuakeWindowWindow是Qt内置用来管理QuakeWindow的Mouse事件的object
    if (watched->objectName() != "QuakeWindowWindow")
        return MainWindow::eventFilter(watched, event);


    // 鼠标点击事件
    if (event->type() == QEvent::MouseButtonPress) {
        // 鼠标点击窗口后激活窗口,因为不走窗管
        activateWindow();
        // 判断是否可以resize
        if (Quake_Prepare_Resize == m_resizeState) {
            // 若可以设置状态为resize,等待用户move鼠标
            m_resizeState = Quake_Resize;
        }
    }

    // 鼠标释放事件
    if (event->type() == QEvent::MouseButtonRelease) {
        // 松开鼠标 将状态值恢复默认
        m_resizeState = Quake_NoResize;
    }
    // 鼠标移动事件
    if (event->type() == QEvent::MouseMove) {
        // 将事件装换为鼠标事件
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event) ;
        int mousePosY = mouseEvent->screenPos().y();
        int margin = mousePosY - height();
        switch (m_resizeState) {
        // 若此时已经准备好resize
        case Quake_Resize:
            // 记录当前的雷神窗口高度
            m_quakeWindowHeight = height() + margin;
            // 延时发送,避免resize过于频繁,导致titlebar抖动
            m_resizeTimer->start(5);
            break;
        // 若没到边缘或者到边缘还没点击
        default:
            // 判断鼠标位置
            if (qAbs(margin) < QUAKE_EDGE) {
                // 若小于定义的边缘大小,将光标变化为指定形状
                setCursor(Qt::SizeVerCursor);
                m_resizeState = Quake_Prepare_Resize;
            } else {
                // 若大于,则还原
                setCursor(Qt::ArrowCursor);
                m_resizeState = Quake_NoResize;
            }
            break;
        }
    }
#endif
    return MainWindow::eventFilter(watched, event);

}

int QuakeWindow::getQuakeHeight()
{
    int screenHeight = qApp->desktop()->screenGeometry().height();
    int minHeight = screenHeight * 1 / 3;
    return m_winInfoConfig->value(CONFIG_QUAKE_WINDOW_HEIGHT, minHeight).toInt();
}

void QuakeWindow::switchEnableResize()
{
    // 如果(桌面光标Y坐标)>(雷神窗口Y坐标+雷神高度的1/2),则启用拉伸属性.否则禁用拉伸属性
    // 此方法dtk已经标记废弃
    switchEnableResize(QCursor::pos().y() > pos().y() + height() / 2);
}

void QuakeWindow::switchEnableResize(bool isEnable)
{
    if (isEnable) {
        // 设置最小高度和最大高度，解放fixSize设置的不允许拉伸
        QDesktopWidget *desktopWidget = QApplication::desktop();
        QRect screenRect = desktopWidget->screenGeometry(); //获取设备屏幕大小
        //Add by ut001000 renfeixiang 2020-11-16 修改成使用写好的设置最小值的函数
        updateMinHeight();
        setMaximumHeight(screenRect.height() * 2 / 3);
    } else {
        // 窗管和DTK让用fixSize来替代，禁止resize
        setFixedHeight(height());
    }
}
