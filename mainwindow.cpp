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

#include "themepanelplugin.h"

#include "customcommandplugin.h"
#include "remotemanagementplugn.h"
#include "serverconfigmanager.h"

#include <DSettings>
#include <DSettingsGroup>
#include <DSettingsOption>
#include <DSettingsWidgetFactory>
#include <DThemeManager>
#include <DTitlebar>

#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMouseEvent>
#include <QProcess>
#include <QSettings>
#include <QShortcut>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

MainWindow::MainWindow(TermProperties properties, QWidget *parent)
    : DMainWindow(parent),
      m_menu(new QMenu),
      m_tabbar(nullptr),
      m_centralWidget(new QWidget(this)),
      m_centralLayout(new QVBoxLayout(m_centralWidget)),
      m_termStackWidget(new QStackedWidget),
      m_titlebarStyleSheet(titlebar()->styleSheet()),
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

void MainWindow::addTab(TermProperties properties, bool activeTab)
{
    TermWidgetPage *termPage = new TermWidgetPage(properties);
    setNewTermPage(termPage, activeTab);

    int index = m_tabbar->addTab(termPage->identifier(), "New Terminal Tab");
    if (activeTab) {
        m_tabbar->setCurrentIndex(index);
    }

    connect(termPage, &TermWidgetPage::termTitleChanged, this, &MainWindow::onTermTitleChanged);
    connect(termPage, &TermWidgetPage::tabTitleChanged, this, &MainWindow::onTabTitleChanged);
    connect(termPage, &TermWidgetPage::termRequestOpenSettings, this, &MainWindow::showSettingDialog);
    connect(termPage, &TermWidgetPage::lastTermClosed, this, &MainWindow::closeTab);
    connect(termPage, &TermWidgetPage::termGetFocus, this, [=]() {
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
    connect(termPage, &TermWidgetPage::termRequestOpenCustomCommand, this, [=]() {
        CustomCommandPlugin *plugin = qobject_cast<CustomCommandPlugin *>(getPluginByName(PLUGIN_TYPE_CUSTOMCOMMAND));
        plugin->getCustomCommandTopPanel()->show();
    });
    connect(termPage, &TermWidgetPage::termRequestOpenRemoteManagement, this, [=]() {
        RemoteManagementPlugn *plugin =
        qobject_cast<RemoteManagementPlugn *>(getPluginByName(PLUGIN_TYPE_REMOTEMANAGEMENT));
        plugin->getRemoteManagementTopPanel()->show();
    });
}

bool showExitConfirmDialog()
{
    OperationConfirmDlg optDlg;
    optDlg.setOperatTypeName(QObject::tr("Programs are still running in terminal"));
    optDlg.setTipInfo(QObject::tr("Are you sure you want to exit?"));
    optDlg.setOKCancelBtnText(QObject::tr("Exit"), QObject::tr("Cancel"));
    optDlg.exec();

    return (optDlg.getConfirmResult() == QDialog::Accepted);
}

void MainWindow::closeTab(const QString &identifier)
{
    for (int i = 0, count = m_termStackWidget->count(); i < count; i++) {
        TermWidgetPage *tabPage = qobject_cast<TermWidgetPage *>(m_termStackWidget->widget(i));
        if (tabPage && tabPage->identifier() == identifier) {
            TermWidgetWrapper *termWidgetWapper = tabPage->currentTerminal();
            if (termWidgetWapper->hasRunningProcess() && !showExitConfirmDialog()) {
                qDebug() << "here are processes running in this terminal tab... " << tabPage->identifier() << endl;
                return;
            }

            m_termStackWidget->removeWidget(tabPage);
            tabPage->deleteLater();
            m_tabbar->removeTab(identifier);
            break;
        }
    }

    if (m_tabbar->count() == 0) {
        qApp->quit();
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
    if (hasRunning && !showExitConfirmDialog()) {
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
    ThemePanelPlugin *testPlugin = new ThemePanelPlugin(this);
    testPlugin->initPlugin(this);

    CustomCommandPlugin *customCommandPlugin = new CustomCommandPlugin(this);
    customCommandPlugin->initPlugin(this);

    RemoteManagementPlugn *remoteManagPlugin = new RemoteManagementPlugn(this);
    remoteManagPlugin->initPlugin(this);

    m_plugins.append(testPlugin);
    m_plugins.append(customCommandPlugin);
    m_plugins.append(remoteManagPlugin);
}

void MainWindow::initWindow()
{
    // 全屏退出按钮
    QIcon ico(":/images/icon/hover/exit_hover.svg");
    m_exitFullScreen = new DPushButton();
    m_exitFullScreen->setIcon(ico);
    m_exitFullScreen->setIconSize(QSize(36, 36));
    m_exitFullScreen->setFixedSize(QSize(36, 36));
    m_exitFullScreen->setFlat(true);
    titlebar()->addWidget(m_exitFullScreen, Qt::AlignRight | Qt::AlignHCenter);
    m_exitFullScreen->setVisible(false);
    connect(m_exitFullScreen, &DPushButton::clicked, this, [this]() { switchFullscreen(); });

    QSettings settings("blumia", "dterm");
    const QString &windowState =
    Settings::instance()->settings->option("advanced.window.use_on_starting")->value().toString();
    // init window state.
    if (windowState == "window_maximum") {
        showMaximized();
    } else if (windowState == "fullscreen") {
        switchFullscreen(true);
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
    // new_tab
    QShortcut *newTab =
    new QShortcut(QKeySequence(Settings::instance()->getKeyshortcutFromKeymap("workspace", "new_workspace")), this);
    connect(newTab, &QShortcut::activated, this, [this]() {
        this->addTab(currentTab()->createCurrentTerminalProperties(), true);
    });

    // CloseWorkspace
    QShortcut *CloseWorkspace =
    new QShortcut(QKeySequence(Settings::instance()->getKeyshortcutFromKeymap("workspace", "close_workspace")), this);
    connect(CloseWorkspace, &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) {
            closeTab(page->identifier());
        }
    });

    // PreWorkspace
    QShortcut *PreWorkspace = new QShortcut(
    QKeySequence(Settings::instance()->getKeyshortcutFromKeymap("workspace", "previous_workspace")), this);
    connect(PreWorkspace, &QShortcut::activated, this, [this]() {
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

    // NextWorkspace
    QShortcut *NextWorkspace =
    new QShortcut(QKeySequence(Settings::instance()->getKeyshortcutFromKeymap("workspace", "next_workspace")), this);
    connect(NextWorkspace, &QShortcut::activated, this, [this]() {
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

    // horizontal_split
    QShortcut *splitHorizontal =
    new QShortcut(QKeySequence(Settings::instance()->getKeyshortcutFromKeymap("workspace", "horionzal_split")), this);
    connect(splitHorizontal, &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) {
            qDebug() << "horizontal_split";
            page->split(Qt::Horizontal);
        }
    });

    // vertical_split
    QShortcut *splitVertical =
    new QShortcut(QKeySequence(Settings::instance()->getKeyshortcutFromKeymap("workspace", "vertical_split")), this);
    connect(splitVertical, &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) {
            qDebug() << "vertical_split";
            page->split(Qt::Vertical);
        }
    });

    // FocusNavUp
    QShortcut *focusNavUp = new QShortcut(
    QKeySequence(Settings::instance()->getKeyshortcutFromKeymap("workspace", "select_upper_window")), this);
    connect(focusNavUp, &QShortcut::activated, this, [this]() {
        qDebug() << "Alt+k";
        TermWidgetPage *page = currentTab();
        if (page) {
            page->focusNavigation(Qt::TopEdge);
        }
    });
    // FocusNavDown
    QShortcut *focusNavDown = new QShortcut(
    QKeySequence(Settings::instance()->getKeyshortcutFromKeymap("workspace", "select_lower_window")), this);
    connect(focusNavDown, &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) {
            page->focusNavigation(Qt::BottomEdge);
        }
    });
    // FocusNavLeft
    QShortcut *focusNavLeft = new QShortcut(
    QKeySequence(Settings::instance()->getKeyshortcutFromKeymap("workspace", "select_left_window")), this);
    connect(focusNavLeft, &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) {
            page->focusNavigation(Qt::LeftEdge);
        }
    });
    // FocusNavRight
    QShortcut *focusNavRight = new QShortcut(
    QKeySequence(Settings::instance()->getKeyshortcutFromKeymap("workspace", "select_right_window")), this);
    connect(focusNavRight, &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) {
            page->focusNavigation(Qt::RightEdge);
            // QMouseEvent e(QEvent::MouseButtonPress, ) QApplication::sendEvent(focusWidget(), &keyPress);
        }
    });

    // CloseWindows
    QShortcut *CloseWindow =
    new QShortcut(QKeySequence(Settings::instance()->getKeyshortcutFromKeymap("workspace", "close_window")), this);
    connect(CloseWindow, &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) {
            qDebug() << "CloseWindow";
            page->closeSplit(page->currentTerminal());
        }
    });

    // CloseOtherWindows
    QShortcut *CloseOtherWindows = new QShortcut(
    QKeySequence(Settings::instance()->getKeyshortcutFromKeymap("workspace", "close_other_windows")), this);
    connect(CloseOtherWindows, &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) {
            page->closeOtherTerminal();
        }
    });

    // copy
    QShortcut *copyShortcut =
    new QShortcut(QKeySequence(Settings::instance()->getKeyshortcutFromKeymap("terminal", "copy")), this);
    connect(copyShortcut, &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) {
            page->copyClipboard();
        }
    });

    // paste
    QShortcut *pasteShortcut =
    new QShortcut(QKeySequence(Settings::instance()->getKeyshortcutFromKeymap("terminal", "paste")), this);
    connect(pasteShortcut, &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) {
            page->pasteClipboard();
        }
        // page->doAction("paste");
    });

    // Search
    QShortcut *SearchShortcut =
    new QShortcut(QKeySequence(Settings::instance()->getKeyshortcutFromKeymap("terminal", "search")), this);
    connect(SearchShortcut, &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) {
            page->toggleShowSearchBar();
        }
    });

    // zoom_in
    QShortcut *zoomInShortcut =
    new QShortcut(QKeySequence(Settings::instance()->getKeyshortcutFromKeymap("terminal", "zoom_in")), this);
    connect(zoomInShortcut, &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) {
            page->zoomInCurrentTierminal();
        }
    });

    // zoom_out
    QShortcut *zoomOutShortcut =
    new QShortcut(QKeySequence(Settings::instance()->getKeyshortcutFromKeymap("terminal", "zoom_out")), this);
    connect(zoomOutShortcut, &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) {
            page->zoomOutCurrentTerminal();
        }
    });

    // DefaultFontSize
    QShortcut *DefaultFontSize =
    new QShortcut(QKeySequence(Settings::instance()->getKeyshortcutFromKeymap("terminal", "default_size")), this);
    connect(DefaultFontSize, &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) {
            // qDebug() << "DefaultFontSize";
            page->setFontSize(Settings::instance()->fontSize());
        }
    });

    // selectAll
    QShortcut *selectAll =
    new QShortcut(QKeySequence(Settings::instance()->getKeyshortcutFromKeymap("terminal", "select_all")), this);
    connect(selectAll, &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) {
            qDebug() << "selectAll";
            page->selectAll();
        }
    });

    // skipToNextCommand
    QShortcut *skipToNextCommand = new QShortcut(
    QKeySequence(Settings::instance()->getKeyshortcutFromKeymap("terminal", "skip_to_next_command")), this);
    connect(skipToNextCommand, &QShortcut::activated, this, [this]() {
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

    // skipToPreCommand
    QShortcut *skipToPreCommand = new QShortcut(
    QKeySequence(Settings::instance()->getKeyshortcutFromKeymap("terminal", "skip_to_previous_command")), this);
    connect(skipToPreCommand, &QShortcut::activated, this, [this]() {
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

    // FullScreen
    QShortcut *fullscreen =
    new QShortcut(QKeySequence(Settings::instance()->getKeyshortcutFromKeymap("advanced", "switch_fullscreen")), this);
    connect(fullscreen, &QShortcut::activated, this, [this]() { switchFullscreen(); });

    // RenameTab
    QShortcut *rename_tab =
    new QShortcut(QKeySequence(Settings::instance()->getKeyshortcutFromKeymap("advanced", "rename_tab")), this);
    connect(rename_tab, &QShortcut::activated, this, [this]() {
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

    // ShowShortCut
    QShortcut *ShowShortCut =
    new QShortcut(QKeySequence(Settings::instance()->getKeyshortcutFromKeymap("advanced", "display_shortcuts")), this);
    connect(ShowShortCut, &QShortcut::activated, this, [this]() {
        qDebug() << "displayShortcuts";
        displayShortcuts();
    });

    // CustomCommand
    QShortcut *CustomCommand =
    new QShortcut(QKeySequence(Settings::instance()->getKeyshortcutFromKeymap("advanced", "custom_command")), this);
    connect(CustomCommand, &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) {
            emit page->termRequestOpenCustomCommand();
        }
    });

    // RemoteManager
    QShortcut *RemoteManager =
    new QShortcut(QKeySequence(Settings::instance()->getKeyshortcutFromKeymap("advanced", "remote_management")), this);
    connect(RemoteManager, &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) {
            RemoteManagementPlugn *remoteplugin =
            qobject_cast<RemoteManagementPlugn *>(getPluginByName(PLUGIN_TYPE_REMOTEMANAGEMENT));

            emit remoteplugin->getRemoteManagementTopPanel()->show();
        }
    });
    /********************* Modify by n014361 wangpeili End ************************/
}

void MainWindow::initConnections()
{
    connect(Settings::instance(), &Settings::opacityChanged, this, [this](qreal opacity) {
        forAllTabPage([opacity](TermWidgetPage *tabPage) { tabPage->setTerminalOpacity(opacity); });
    });

    connect(Settings::instance(), &Settings::backgroundBlurChanged, this, [this](bool enabled) {
        setEnableBlurWindow(enabled);
    });

    /******** Modify by n014361 wangpeili 2020-01-06:  增加修改字体，修改字体大小，修改光标，光标闪烁功能**/
    connect(Settings::instance(), &Settings::fontSizeChanged, this, [this](int fontsize) {
        forAllTabPage([fontsize](TermWidgetPage *tabPage) { tabPage->setFontSize(fontsize); });
    });

    connect(Settings::instance(), &Settings::fontChanged, this, [this](QString fontName) {
        forAllTabPage([fontName](TermWidgetPage *tabPage) { tabPage->setFont(fontName); });
    });

    connect(Settings::instance(), &Settings::cursorShapeChanged, this, [this](int value) {
        forAllTabPage([value](TermWidgetPage *tabPage) { tabPage->setcursorShape(value); });
    });

    connect(Settings::instance(), &Settings::cursorBlinkChanged, this, [this](bool enable) {
        for (int i = 0, count = m_termStackWidget->count(); i < count; i++) {
            TermWidgetPage *tabPage = qobject_cast<TermWidgetPage *>(m_termStackWidget->widget(i));
            if (tabPage) {
                tabPage->setBlinkingCursor(enable);
            }
        }
        focusTab(currentTab()->identifier());
    });

    connect(Settings::instance(), &Settings::pressingScrollChanged, this, [this](bool value) {
        forAllTabPage([value](TermWidgetPage *tabPage) { tabPage->setPressingScroll(value); });
    });

    /********************* Modify by n014361 wangpeili End ************************/
}

void MainWindow::initTitleBar()
{
    for (MainWindowPluginInterface *plugin : m_plugins) {
        QAction *pluginMenu = plugin->titlebarMenu(this);
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
    }

    connect(m_tabbar, &DTabBar::tabFull, this, [=]() { qDebug() << "tab is full"; });

    connect(m_tabbar,
            &DTabBar::tabCloseRequested,
            this,
            [this](int index) { closeTab(m_tabbar->identifier(index)); },
            Qt::QueuedConnection);

    connect(m_tabbar,
            &DTabBar::tabAddRequested,
            this,
            [this]() { addTab(currentTab()->createCurrentTerminalProperties(), true); },
            Qt::QueuedConnection);

    connect(m_tabbar,
            &DTabBar::currentChanged,
            this,
            [this](int index) { focusTab(m_tabbar->identifier(index)); },
            Qt::QueuedConnection);

    connect(settingAction, &QAction::triggered, this, &MainWindow::showSettingDialog);
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
         Settings::instance()->settings->group(groupname)->options())  // Settings::instance()->settings->keys())
    {
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
