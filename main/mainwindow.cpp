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

#include "encodeplugin/encodepanelplugin.h"

#include "customcommandplugin.h"
#include "serverconfigmanager.h"
#include "utils.h"

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

#include <QVBoxLayout>
#include <QMap>

DWIDGET_USE_NAMESPACE

MainWindow::MainWindow(TermProperties properties, QWidget *parent)
    : DMainWindow(parent),
      m_menu(new QMenu),
      m_tabbar(nullptr),
      m_centralWidget(new QWidget(this)),
      m_centralLayout(new QVBoxLayout(m_centralWidget)),
      m_termStackWidget(new QStackedWidget),
      //m_titlebarStyleSheet(titlebar()->styleSheet()),
      m_properties(properties)
{
    initUI();
}

void MainWindow::initUI()
{
    m_tabbar = new TabBar(this, true);
    m_tabbar->setChromeTabStyle(true);

    // ShortcutManager::instance();不管是懒汉式还是饿汉式，都不需要这样单独操作
    ShortcutManager::instance()->setMainWindow(this);
    m_shortcutManager = ShortcutManager::instance();
    ServerConfigManager::instance()->initServerConfig();
    setAttribute(Qt::WA_TranslucentBackground);

    setMinimumSize(450, 250);
    setCentralWidget(m_centralWidget);
    setWindowIcon(QIcon::fromTheme("deepin-terminal"));

    // Init layout
    m_centralLayout->setMargin(0);
    m_centralLayout->setSpacing(0);

    m_centralLayout->addWidget(m_termStackWidget);

    initConnections();
    initShortcuts();

    // Plugin may need centralWidget() to work so make sure initPlugin() is after setCentralWidget()
    // Other place (eg. create titlebar menu) will call plugin method so we should create plugins before init other
    // parts.
    initPlugins();
    initWindow();
    qDebug() << m_termStackWidget->size();
    qApp->installEventFilter(this);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setQuakeWindow(bool isQuakeWindow)
{
    m_isQuakeWindow = isQuakeWindow;
    if (m_isQuakeWindow) {
        titlebar()->setFixedHeight(0);
        setWindowRadius(0);

        QRect deskRect = QApplication::desktop()->availableGeometry();

        Qt::WindowFlags windowFlags = this->windowFlags();
        setWindowFlags(windowFlags | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool);
        /******** Modify by n014361 wangpeili 2020-02-18: 全屏设置后，启动雷神窗口要强制取消****************/
        setWindowState(windowState() & ~Qt::WindowFullScreen);
        /********************* Modify by n014361 wangpeili End ************************/
        this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        this->setMinimumSize(deskRect.size().width(), 60);
        this->resize(deskRect.size().width(), 200);
        this->move(0, 0);
    }

    initTitleBar();
    addTab(m_properties);
}

bool MainWindow::isTabVisited(int tabSessionId)
{
    return m_tabVisitMap.value(tabSessionId);
}

void MainWindow::addTab(TermProperties properties, bool activeTab)
{
    TermWidgetPage *termPage = new TermWidgetPage(properties, this);
    setNewTermPage(termPage, activeTab);

    int index = m_tabbar->addTab(termPage->identifier(), "New Terminal Tab");
    if (activeTab) {
        m_tabbar->setCurrentIndex(index);
    }

    TermWidgetWrapper *termWidgetWapper = termPage->currentTerminal();
    m_tabbar->saveSessionIdWithTabIndex(termWidgetWapper->getCurrSessionId(), index);
    m_tabbar->saveSessionIdWithTabId(termWidgetWapper->getCurrSessionId(), termPage->identifier());

    connect(termPage, &TermWidgetPage::pageRequestNewWorkspace, this, [this]() {
        this->addTab(currentTab()->createCurrentTerminalProperties(), true);
    });
    connect(termPage, &TermWidgetPage::pageRequestShowEncoding, this, [this]() {
        EncodePanelPlugin *plugin = qobject_cast<EncodePanelPlugin *>(getPluginByName(PLUGIN_TYPE_ENCODING));
        if (plugin) {
            plugin->getEncodePanel()->show();
        }
    });
    connect(termPage, &TermWidgetPage::termTitleChanged, this, &MainWindow::onTermTitleChanged);
    connect(termPage, &TermWidgetPage::tabTitleChanged, this, &MainWindow::onTabTitleChanged);
    connect(termPage, &TermWidgetPage::termRequestOpenSettings, this, &MainWindow::showSettingDialog);
    connect(termPage, &TermWidgetPage::lastTermClosed, this, &MainWindow::closeTab);
    connect(termPage, &TermWidgetPage::termGetFocus, this, [ = ]() {
        CustomCommandPlugin *plugin = qobject_cast<CustomCommandPlugin *>(getPluginByName(PLUGIN_TYPE_CUSTOMCOMMAND));
        if (plugin) {
            emit plugin->doHide();
        }
        RemoteManagementPlugn *remoteMgtPlugin =
            qobject_cast<RemoteManagementPlugn *>(getPluginByName(PLUGIN_TYPE_REMOTEMANAGEMENT));
        if (remoteMgtPlugin) {
            emit remoteMgtPlugin->doHide();
        }
    });
    connect(termPage, &TermWidgetPage::termRequestOpenCustomCommand, this, [ = ]() {
        CustomCommandPlugin *plugin = qobject_cast<CustomCommandPlugin *>(getPluginByName(PLUGIN_TYPE_CUSTOMCOMMAND));
        plugin->getCustomCommandTopPanel()->show();
    });
    connect(termPage, &TermWidgetPage::termRequestOpenRemoteManagement, this, [ = ]() {
        RemoteManagementPlugn *plugin =
            qobject_cast<RemoteManagementPlugn *>(getPluginByName(PLUGIN_TYPE_REMOTEMANAGEMENT));
        plugin->getRemoteManagementTopPanel()->show();
    });

    connect(termPage->currentTerminal(), &TermWidgetWrapper::termIsIdle, this, [ = ](int currSessionId, bool bIdle) {

        int tabIndex = m_tabbar->queryIndexBySessionId(currSessionId);

        if (isTabVisited(currSessionId) && !bIdle) {
            DGuiApplicationHelper *appHelper = DGuiApplicationHelper::instance();
            DPalette pa = appHelper->standardPalette(appHelper->themeType());
            m_tabbar->setClearTabColor(tabIndex);
            return;
        }

        if (bIdle) {
            DGuiApplicationHelper *appHelper = DGuiApplicationHelper::instance();
            DPalette pa = appHelper->standardPalette(appHelper->themeType());
            m_tabbar->setClearTabColor(tabIndex);

            if (isTabVisited(currSessionId)) {
                m_tabVisitMap.insert(currSessionId, false);
            }
        } else {
            DGuiApplicationHelper *appHelper = DGuiApplicationHelper::instance();
            DPalette pa = appHelper->standardPalette(appHelper->themeType());
            m_tabbar->setTabTextColor(tabIndex, pa.color(DPalette::Highlight));
        }
    });

    connect(termPage, &TermWidgetPage::termRequestUploadFile, this, &MainWindow::remoteUploadFile);
    connect(termPage, &TermWidgetPage::termRequestDownloadFile, this, &MainWindow::remoteDownloadFile);
}

void MainWindow::closeTab(const QString &identifier)
{
    for (int i = 0, count = m_termStackWidget->count(); i < count; i++) {
        TermWidgetPage *tabPage = qobject_cast<TermWidgetPage *>(m_termStackWidget->widget(i));
        if (tabPage && tabPage->identifier() == identifier) {
            TermWidgetWrapper *termWidgetWapper = tabPage->currentTerminal();
            if (termWidgetWapper->hasRunningProcess() && !Utils::showExitConfirmDialog()) {
                qDebug() << "here are processes running in this terminal tab... " << tabPage->identifier() << endl;
                return;
            }

            m_termStackWidget->removeWidget(tabPage);
            tabPage->deleteLater();
            m_tabbar->removeTab(identifier);

            updateTabStatus();
            break;
        }
    }

    if (m_tabbar->count() == 0) {
        qApp->quit();
    }
}

void MainWindow::updateTabStatus()
{
    for (int i = 0; i < m_tabbar->count(); i++) {
        int currIndex = i;
        TermWidgetPage *tabPage = qobject_cast<TermWidgetPage *>(m_termStackWidget->widget(currIndex));
        TermWidgetWrapper *termWidgetWapper = tabPage->currentTerminal();
        bool bIdle = !(termWidgetWapper->hasRunningProcess());
        int currSessionId = termWidgetWapper->getCurrSessionId();

        if (bIdle) {
            DGuiApplicationHelper *appHelper = DGuiApplicationHelper::instance();
            DPalette pa = appHelper->standardPalette(appHelper->themeType());
            m_tabbar->setClearTabColor(currIndex);
        } else {
            if (isTabVisited(currSessionId)) {
                DGuiApplicationHelper *appHelper = DGuiApplicationHelper::instance();
                DPalette pa = appHelper->standardPalette(appHelper->themeType());
                m_tabbar->setClearTabColor(currIndex);
            } else {
                DGuiApplicationHelper *appHelper = DGuiApplicationHelper::instance();
                DPalette pa = appHelper->standardPalette(appHelper->themeType());
                m_tabbar->setTabTextColor(currIndex, pa.color(DPalette::Highlight));
            }
        }
    }
}

/*******************************************************************************
 1. @函数:     closeOtherTab
 2. @作者:     n014361 王培利
 3. @日期:     2020-01-10
 4. @说明:     关闭其它标签页功能
*******************************************************************************/
void MainWindow::closeOtherTab()
{
    TermWidgetPage *page = currentTab();
    int index = 0;
    while (1 < m_termStackWidget->count()) {
        TermWidgetPage *tabPage = qobject_cast<TermWidgetPage *>(m_termStackWidget->widget(index));
        if (tabPage && tabPage->identifier() != page->identifier()) {
            m_termStackWidget->removeWidget(tabPage);
            m_tabbar->removeTab(tabPage->identifier());

            delete tabPage;
            index = 0;
            // break;
        } else {
            index++;
        }
    }
}

void MainWindow::focusTab(const QString &identifier)
{
    for (int i = 0, count = m_termStackWidget->count(); i < count; i++) {
        TermWidgetPage *tabPage = qobject_cast<TermWidgetPage *>(m_termStackWidget->widget(i));
        if (tabPage && tabPage->identifier() == identifier) {
            m_termStackWidget->setCurrentWidget(tabPage);
            tabPage->focusCurrentTerm();

            break;
        }
    }
}

TermWidgetPage *MainWindow::currentTab()
{
    return qobject_cast<TermWidgetPage *>(m_termStackWidget->currentWidget());
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

void MainWindow::setTitleBarBackgroundColor(QString color)
{
    // how dde-file-manager make the setting dialog works under dark theme?
    if (QColor(color).lightness() < 128) {
        DThemeManager::instance()->setTheme("dark");
    } else {
        DThemeManager::instance()->setTheme("light");
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    if (m_isQuakeWindow) {
        QRect deskRect = QApplication::desktop()->availableGeometry();
        this->resize(deskRect.size().width(), this->size().height());
        this->move(0, 0);
    }
    if (customCommandPlugin) {
        customCommandPlugin->hidePlugn();
    }
    if (remoteManagPlugin) {
        remoteManagPlugin->hidePlugn();
    }
    DMainWindow::resizeEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings("blumia", "dterm");
    settings.setValue("geometry", saveGeometry());

    if (Qt::WindowNoState == windowState()) {
        settings.setValue("save_width", this->width());
        settings.setValue("save_height", this->height());
    }
    settings.setValue("windowState", saveState());
    bool hasRunning = closeProtect();
    if (hasRunning && !Utils::showExitConfirmDialog()) {
        qDebug() << "close window protect..." << endl;
        event->ignore();
        return;
    }

    DMainWindow::closeEvent(event);
}

bool MainWindow::closeProtect()
{
    // Do not ask for confirmation during log out and power off
    if (qApp->isSavingSession()) {
        return false;
    }

    // Check what processes are running, excluding the shell
    TermWidgetPage *tabPage = currentTab();
    TermWidgetWrapper *termWidgetWapper = tabPage->currentTerminal();
    QList<int> processesRunning = termWidgetWapper->getRunningSessionIdList();
    qDebug() << processesRunning << endl;

    // Get number of open tabs
    const int openTabs = m_termStackWidget->count();

    // If no processes running (except the shell) and no extra tabs, just close
    if (processesRunning.count() == 0 && openTabs < 2) {
        return false;
    }

    // make sure the window is shown on current desktop and is not minimized
    if (this->isMinimized() && !this->m_isQuakeWindow) {
        qDebug() << "isMinimized........... " << endl;
        this->setWindowState((this->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    }

    if (processesRunning.count() > 0) {
        qDebug() << "here are " << processesRunning.count() << " processes running in this window. " << endl;
        return true;
    }

    return false;
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
        titlebar()->addWidget(m_exitFullScreen, Qt::AlignRight | Qt::AlignHCenter);
        m_exitFullScreen->setVisible(true);
        titlebar()->setMenuVisible(false);

        window()->setWindowState(windowState() | Qt::WindowFullScreen);

    } else {
        titlebar()->removeWidget(m_exitFullScreen);
        m_exitFullScreen->setVisible(false);
        titlebar()->setMenuVisible(true);
        window()->setWindowState(windowState() & ~Qt::WindowFullScreen);
    }
    // displayTitlebarIcon();
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
    m_tabbar->setTabText(tabPage->identifier(), title);
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

void MainWindow::initWindow()
{
    // 全屏退出按钮
    QIcon ico(":/resources/images/icon/hover/exit_hover.svg");
    m_exitFullScreen = new DPushButton();
    applyTheme();
    m_exitFullScreen->setIconSize(QSize(36, 36));
    m_exitFullScreen->setFixedSize(QSize(36, 36));
    m_exitFullScreen->setFlat(true);
    titlebar()->addWidget(m_exitFullScreen, Qt::AlignRight | Qt::AlignHCenter);
    m_exitFullScreen->setVisible(false);
    connect(m_exitFullScreen, &DPushButton::clicked, this, [this]() { switchFullscreen(); });

    QSettings settings("blumia", "dterm");

    QString windowState =
        Settings::instance()->settings->option("advanced.window.use_on_starting")->value().toString();
    if (m_properties.contains(StartWindowState)) {
        QString state = m_properties[StartWindowState].toString();
        qDebug() << "use line state set:" << state;
        if (state == "maximize") {
            windowState = "window_maximum";
        } else if (state == "halfscreen") {
            windowState = "Halfscreen";
        } else if ((state == "fullscreen") || (state == "normal")) {
            windowState = state;
        } else {
            qDebug() << "error line state set:" << state << "ignore it!";
        }
    }

    // init window state.
    if (windowState == "window_maximum") {
        showMaximized();
    } else if (windowState == "fullscreen") {
        switchFullscreen(true);
        /******** Modify by n014361 wangpeili 2020-02-25:增加半屏设置    ****************/
    } else if (windowState == "Halfscreen") {
        setWindowRadius(0);
        resize(QSize(QApplication::desktop()->width() / 2, QApplication::desktop()->height()));
        move(0, 0);
        /********************* Modify by n014361 wangpeili End ************************/
    } else {
        resize(QSize(settings.value("save_width").toInt(), settings.value("save_height").toInt()));
        move((QApplication::desktop()->width() - width()) / 2, (QApplication::desktop()->height() - height()) / 2);
    }
    setEnableBlurWindow(Settings::instance()->backgroundBlur());
}

void MainWindow::initShortcuts()
{
    m_shortcutManager->initShortcuts();
    /******** Modify by n014361 wangpeili 2020-01-10: 增加设置的各种快捷键修改关联***********×****/
    // new_workspace
    connect(createNewShotcut("shortcuts.workspace.new_workspace"), &QShortcut::activated, this, [this]() {
        this->addTab(currentTab()->createCurrentTerminalProperties(), true);
    });

    // close_workspace
    connect(createNewShotcut("shortcuts.workspace.close_workspace"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) {
            closeTab(page->identifier());
        }
    });

    // previous_workspace
    connect(createNewShotcut("shortcuts.workspace.previous_workspace"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
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
        TermWidgetPage *page = currentTab();
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
    connect(createNewShotcut("shortcuts.workspace.horionzal_split"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) {
            qDebug() << "horizontal_split";
            page->split(Qt::Horizontal);
        }
    });

    // vertical_split
    connect(createNewShotcut("shortcuts.workspace.vertical_split"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) {
            qDebug() << "vertical_split";
            page->split(Qt::Vertical);
        }
    });

    // select_upper_window
    connect(createNewShotcut("shortcuts.workspace.select_upper_window"), &QShortcut::activated, this, [this]() {
        qDebug() << "Alt+k";
        TermWidgetPage *page = currentTab();
        if (page) {
            page->focusNavigation(Qt::TopEdge);
        }
    });
    // select_lower_window
    connect(createNewShotcut("shortcuts.workspace.select_lower_window"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) {
            page->focusNavigation(Qt::BottomEdge);
        }
    });
    // select_left_window
    connect(createNewShotcut("shortcuts.workspace.select_left_window"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) {
            page->focusNavigation(Qt::LeftEdge);
        }
    });
    // select_right_window
    connect(createNewShotcut("shortcuts.workspace.select_right_window"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) {
            page->focusNavigation(Qt::RightEdge);
            // QMouseEvent e(QEvent::MouseButtonPress, ) QApplication::sendEvent(focusWidget(), &keyPress);
        }
    });

    // close_window
    connect(createNewShotcut("shortcuts.workspace.close_window"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) {
            qDebug() << "CloseWindow";
            page->closeSplit(page->currentTerminal());
        }
    });

    // close_other_windows
    connect(createNewShotcut("shortcuts.workspace.close_other_windows"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) {
            page->closeOtherTerminal();
        }
    });

    // copy
    connect(createNewShotcut("shortcuts.terminal.copy"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) {
            page->copyClipboard();
        }
    });

    // paste
    connect(createNewShotcut("shortcuts.terminal.paste"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) {
            page->pasteClipboard();
        }
        // page->doAction("paste");
    });

    // search
    connect(createNewShotcut("shortcuts.terminal.search"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) {
            page->showSearchBar(true);
        }
    });

    // zoom_in
    connect(createNewShotcut("shortcuts.terminal.zoom_in"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) {
            page->zoomInCurrentTierminal();
        }
    });

    // zoom_out
    connect(createNewShotcut("shortcuts.terminal.zoom_out"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) {
            page->zoomOutCurrentTerminal();
        }
    });

    // default_size
    connect(createNewShotcut("shortcuts.terminal.default_size"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) {
            page->setFontSize(Settings::instance()->fontSize());
        }
    });

    // select_all
    connect(createNewShotcut("shortcuts.terminal.select_all"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) {
            qDebug() << "selectAll";
            page->selectAll();
        }
    });

    // skip_to_next_command
    connect(createNewShotcut("shortcuts.terminal.skip_to_next_command"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) {
            qDebug() << "skipToNextCommand???";
            // page->skipToPreCommand();
            QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
            QApplication::sendEvent(focusWidget(), &keyPress);

            QKeyEvent keyRelease(QEvent::KeyRelease, Qt::Key_Down, Qt::NoModifier);
            QApplication::sendEvent(focusWidget(), &keyRelease);
        }
    });

    // skip_to_previous_command
    connect(createNewShotcut("shortcuts.terminal.skip_to_previous_command"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) {
            qDebug() << "skipToPreCommand";
            QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
            QApplication::sendEvent(focusWidget(), &keyPress);

            QKeyEvent keyRelease(QEvent::KeyRelease, Qt::Key_Up, Qt::NoModifier);
            QApplication::sendEvent(focusWidget(), &keyRelease);

            // m_menu->show();
        }
    });

    // switch_fullscreen
    connect(createNewShotcut("shortcuts.advanced.switch_fullscreen"), &QShortcut::activated, this, [this]() {
        switchFullscreen();
    });

    // rename_tab
    connect(createNewShotcut("shortcuts.advanced.rename_tab"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) {
            bool ok;
            QString text =
                DInputDialog::getText(nullptr, tr("Rename Tab"), tr("Tab name:"), QLineEdit::Normal, QString(), &ok);
            if (ok) {
                page->onTermRequestRenameTab(text);
            }
        }
    });

    // display_shortcuts
    connect(createNewShotcut("shortcuts.advanced.display_shortcuts"), &QShortcut::activated, this, [this]() {
        qDebug() << "displayShortcuts";
        displayShortcuts();
    });

    // custom_command
    connect(createNewShotcut("shortcuts.advanced.custom_command"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) {
            emit page->termRequestOpenCustomCommand();
        }
    });

    // remote_management
    connect(createNewShotcut("shortcuts.advanced.remote_management"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) {
            RemoteManagementPlugn *remoteplugin =
                qobject_cast<RemoteManagementPlugn *>(getPluginByName(PLUGIN_TYPE_REMOTEMANAGEMENT));

            emit remoteplugin->getRemoteManagementTopPanel()->show();
        }
    });
    /********************* Modify by n014361 wangpeili End ************************/

    // ctrl+5 not response?
    for (int i = 1; i <= 9; i++) {
        QString shortCutStr = QString("alt+%1").arg(i);
        qDebug() << shortCutStr;
        QShortcut *switchTabSC = new QShortcut(QKeySequence(shortCutStr), this);
        connect(switchTabSC, &QShortcut::activated, this, [this, i]() {
            qDebug() << i << endl;

            TermWidgetPage *page = currentTab();

            if (!page) {
                return;
            }

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
        });
    }
}

void MainWindow::initConnections()
{
    connect(Settings::instance(), &Settings::windowSettingChanged, this, &MainWindow::onWindowSettingChanged);
    connect(Settings::instance(), &Settings::shortcutSettingChanged, this, &MainWindow::onShortcutSettingChanged);

    connect(this, &MainWindow::newWindowRequest, this, &MainWindow::onCreateNewWindow);

    connect(DApplicationHelper::instance(), &DApplicationHelper::themeTypeChanged, this, [ = ]() { applyTheme(); });
}

void MainWindow::initTitleBar()
{
    QAction *newWorkspaceAction(new QAction(tr("New &workspace"), this));
    connect(newWorkspaceAction, &QAction::triggered, this, [this]() {
        this->addTab(currentTab()->createCurrentTerminalProperties(), true);
    });
    m_menu->addAction(newWorkspaceAction);
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

    m_titleBar = new TitleBar(this, m_isQuakeWindow);
    m_titleBar->setTabBar(m_tabbar);
    if (m_isQuakeWindow) {
        m_titleBar->setBackgroundRole(QPalette::Window);
        m_titleBar->setAutoFillBackground(true);
        m_centralLayout->addWidget(m_titleBar);
    } else {
        titlebar()->setCustomWidget(m_titleBar);
        titlebar()->setAutoHideOnFullscreen(true);
        titlebar()->setMenu(m_menu);
        titlebar()->setTitle("");

        //设置titlebar焦点策略为不抢占焦点策略，防止点击titlebar后终端失去输入焦点
        titlebar()->setFocusPolicy(Qt::NoFocus);
    }

    connect(m_tabbar, &DTabBar::tabFull, this, [ = ]() {
        qDebug() << "tab is full";

        TermWidgetPage *tabPage = currentTab();
        TermWidgetWrapper *termWidgetWapper = tabPage->currentTerminal();
        QString currWorkingDir = termWidgetWapper->workingDirectory();
        emit newWindowRequest(currWorkingDir);
    });

    connect(m_tabbar, &DTabBar::tabBarClicked, this, [ = ](int index) {

        DGuiApplicationHelper *appHelper = DGuiApplicationHelper::instance();
        DPalette pa = appHelper->standardPalette(appHelper->themeType());
        m_tabbar->setClearTabColor(index);

        TermWidgetPage *tabPage = qobject_cast<TermWidgetPage *>(m_termStackWidget->widget(index));
        TermWidgetWrapper *termWidgetWapper = tabPage->currentTerminal();
        if (termWidgetWapper->hasRunningProcess()) {
            m_tabVisitMap.insert(termWidgetWapper->getCurrSessionId(), true);
        }
    });

    connect(m_tabbar, &DTabBar::tabCloseRequested, this, [this](int index) {
        closeTab(m_tabbar->identifier(index));
    }, Qt::QueuedConnection);

    connect(m_tabbar, &DTabBar::tabAddRequested, this, [this]() {
        addTab(currentTab()->createCurrentTerminalProperties(), true);
    }, Qt::QueuedConnection);

    connect(m_tabbar, &DTabBar::currentChanged, this, [this](int index) {
        focusTab(m_tabbar->identifier(index));
    }, Qt::QueuedConnection);

    connect(m_tabbar, &TabBar::closeTabs, this, [ = ](QList<QString> closeTabIdList) {

        QMap<QString, TermWidgetPage *> pageMap;
        for (int i = 0, count = m_termStackWidget->count(); i < count; i++) {
            TermWidgetPage *tabPage = qobject_cast<TermWidgetPage *>(m_termStackWidget->widget(i));
            pageMap.insert(tabPage->identifier(), tabPage);
        }

        for (int i = 0; i < closeTabIdList.size(); i++) {
            QString tabIdentifier = closeTabIdList.at(i);

            TermWidgetPage *tabPage = pageMap.value(tabIdentifier);
            if (tabPage) {
                TermWidgetWrapper *termWidgetWapper = tabPage->currentTerminal();
                if (termWidgetWapper->hasRunningProcess() && !Utils::showExitConfirmDialog()) {
                    qDebug() << "here are processes running in this terminal tab... " << tabPage->identifier() << endl;
                    return;
                }
            }
        }

        for (int i = 0; i < closeTabIdList.size(); i++) {

            QString tabIdentifier = closeTabIdList.at(i);
            TermWidgetPage *tabPage = pageMap.value(tabIdentifier);
            m_tabbar->removeTab(tabIdentifier);
            if (tabPage && tabPage->identifier() == tabIdentifier) {
                m_termStackWidget->removeWidget(tabPage);
                delete tabPage;
            }
        }
    });

    connect(settingAction, &QAction::triggered, this, &MainWindow::showSettingDialog);
}

void MainWindow::onCreateNewWindow(QString workingDir)
{
    qDebug() << "************************workingDir:" << workingDir << endl;
    TermProperties properties;
    properties.setWorkingDir(workingDir);
    MainWindow *window = new MainWindow(properties);
    window->setQuakeWindow(false);
    window->show();
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (m_isQuakeWindow) {
        // disable move window
        if (event->type() == QEvent::MouseMove || event->type() == QEvent::DragMove) {
            if (watched->objectName() == QLatin1String("QMainWindowClassWindow")) {
                event->ignore();
                return true;
            }
        }

        /********** Modify by n013252 wangliang 2020-01-14: 雷神窗口从未激活状态恢复****************/
        if (watched == this && event->type() == QEvent::WindowStateChange) {
            event->ignore();

            show();
            raise();
            activateWindow();

            m_isQuakeWindowActivated = true;
            return true;
        }
        /**************** Modify by n013252 wangliang End ****************/

        /******** Modify by n014361 wangpeili 2020-01-13:雷神模式隐藏 ****************/
        if (Settings::instance()->settings->option("advanced.window.auto_hide_raytheon_window")->value().toBool()) {
            if (watched == this && event->type() == QEvent::WindowDeactivate) {
                qDebug() << "WindowDeactivate" << event->type();
                this->hide();
                this->setQuakeWindowActivated(false);
            }
        }
        /********************* Modify by n014361 wangpeili End ************************/
    }

    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            if (enterSzCommand) {
                executeDownloadFile();
                enterSzCommand = false;
            }
        }
        if ((keyEvent->modifiers() == Qt::ControlModifier) && (keyEvent->key() == Qt::Key_C || keyEvent->key() == Qt::Key_D)) {
            enterSzCommand = false;
        }
    }

    return QObject::eventFilter(watched, event);
}

bool MainWindow::isQuakeWindowActivated()
{
    return m_isQuakeWindowActivated;
}

void MainWindow::setQuakeWindowActivated(bool isQuakeWindowActivated)
{
    m_isQuakeWindowActivated = isQuakeWindowActivated;
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
    // auto_hide_raytheon_window在使用中自动读取生效
    // use_on_starting重启生效
    if ((keyName == "advanced.window.auto_hide_raytheon_window") || (keyName == "advanced.window.use_on_starting")) {
        qDebug() << "settingValue[" << keyName << "] changed to " << Settings::instance()->OutputtingScroll()
                 << ", auto effective when happen";
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

void MainWindow::setNewTermPage(TermWidgetPage *termPage, bool activePage)
{
    m_termStackWidget->addWidget(termPage);
    if (activePage) {
        m_termStackWidget->setCurrentWidget(termPage);
    }
}

void MainWindow::showSettingDialog()
{
    DSettingsDialog *dialog = new DSettingsDialog(this);
    dialog->widgetFactory()->registerWidget("fontcombobox", Settings::createFontComBoBoxHandle);
    dialog->widgetFactory()->registerWidget("slider", Settings::createCustomSliderHandle);
    dialog->updateSettings(Settings::instance()->settings);
    dialog->exec();
    delete dialog;

    /******** Modify by n014361 wangpeili 2020-01-10:修复显示完设置框以后，丢失焦点的问题*/
    TermWidgetPage *page = currentTab();
    if (page) {
        page->focusCurrentTerm();
    }
    /********************* Modify by n014361 wangpeili End ************************/
}
/*******************************************************************************
 1. @函数:    applyTheme
 2. @作者:    n014361 王培利
 3. @日期:    2020-03-09
 4. @说明:    非DTK控件手动匹配系统主题的修改
*******************************************************************************/
void MainWindow::applyTheme()
{
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
        m_exitFullScreen->setIcon(QIcon(":/resources/images/icon/hover/exit_hover.svg"));
    } else {
        m_exitFullScreen->setIcon(QIcon(":/resources/images/icon/hover/exit_hover_dark.svg"));
    }
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

    QJsonObject workspaceJsonGroup;
    if (keyCategory == "workspace") {
        workspaceJsonGroup.insert("groupName", tr("workspace"));
    } else if (keyCategory == "terminal") {
        workspaceJsonGroup.insert("groupName", tr("terminal"));
    } else if (keyCategory == "advanced") {
        workspaceJsonGroup.insert("groupName", tr("advanced"));
    } else {
        return;
    }
    QString groupname = "shortcuts." + keyCategory;

    QJsonArray JsonArry;
    for (auto opt :
            Settings::instance()->settings->group(groupname)->options()) {  // Settings::instance()->settings->keys())
        QJsonObject jsonItem;
        jsonItem.insert("name", tr(opt->name().toUtf8().data()));
        jsonItem.insert("value", opt->value().toString());
        JsonArry.append(jsonItem);
    }
    QJsonObject JsonGroup;
    JsonGroup.insert("groupName", keyCategory);
    JsonGroup.insert("groupItems", JsonArry);
    jsonGroups.append(JsonGroup);
}
/*******************************************************************************
 1. @函数:    createNewShotcut
 2. @作者:    n014361 王培利
 3. @日期:    2020-02-20
 4. @说明:    创建内置快捷键管理
*******************************************************************************/
QShortcut *MainWindow::createNewShotcut(const QString &key)
{
    QString value = Settings::instance()->settings->option(key)->value().toString();
    QShortcut *shortcut = new QShortcut(QKeySequence(value), this);
    m_BuiltInShortcut[key] = shortcut;
    // qDebug() << "createNewShotcut" << key << value;
    return shortcut;
}


/**
 * Upload file to remote server
 */
void MainWindow::remoteUploadFile()
{
    QString fileName = showFileDailog(false);
    if (!fileName.isNull() && !fileName.isEmpty()) {
        pressCtrlAt();
        sleep(100);
        QString strTxt = "sz '" + fileName + "'\n";
        currentTab()->sendTextToCurrentTerm(strTxt);
    }
}

/**
 * Download file from remote server
 */
void MainWindow::remoteDownloadFile()
{
    downloadFilePath = showFileDailog(true);
    if (!downloadFilePath.isNull() && !downloadFilePath.isEmpty()) {
        QString strTxt = "read -e -a files -p \"" + tr("Type path to download file: ") + "\"; sz \"${files[@]}\"\n";
        currentTab()->sendTextToCurrentTerm(strTxt);
        enterSzCommand = true;
        sleep(100);
    }
}

/**
 * after sz command,wait input file and download file.
 */
void MainWindow::executeDownloadFile()
{
    sleep(1000);
    pressCtrlAt();
    sleep(100);
    QString strCd = "cd " + downloadFilePath + "\n";
    currentTab()->sendTextToCurrentTerm(strCd);
    QString strRz = "rz\n";
    currentTab()->sendTextToCurrentTerm(strRz);
    //if need?
    sleep(500);
    QString strEnter = "\n";
    currentTab()->sendTextToCurrentTerm(strEnter);
    downloadFilePath = "";
}

/**
 * Open file dialog
 */
QString MainWindow::showFileDailog(bool isDir)
{
    QString curPath = QDir::currentPath();
    QString dlgTitle = tr("Select file to upload");
    if (isDir) {
        dlgTitle = tr("Select directory to save the file");
        return DFileDialog::getExistingDirectory(this, dlgTitle, curPath);
    }
    return DFileDialog::getOpenFileName(this, dlgTitle, curPath);
}

void MainWindow::pressCtrlAt()
{
    QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_At, Qt::ControlModifier);
    QApplication::sendEvent(focusWidget(), &keyPress);
}

void MainWindow::sleep(unsigned int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while (QTime::currentTime() < dieTime) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
}
