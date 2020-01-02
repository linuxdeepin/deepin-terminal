#include "mainwindow.h"

#include "titlebar.h"
#include "tabbar.h"
#include "settings.h"
#include "termwidgetpage.h"
#include "termproperties.h"
#include "shortcutmanager.h"
#include "mainwindowplugininterface.h"

#include "themepanelplugin.h"

#include <DTitlebar>
#include <DThemeManager>
#include <QVBoxLayout>
#include <QDebug>
#include <QApplication>
#include <QShortcut>
#include <QSettings>
#include "customcommandplugin.h"
#include "remotemanagementplugn.h"
#include "serverconfigmanager.h"
DWIDGET_USE_NAMESPACE

MainWindow::MainWindow(TermProperties properties, QWidget *parent) :
    DMainWindow(parent),
    m_menu(new QMenu),
    m_tabbar(new TabBar),
    m_centralWidget(new QWidget(this)),
    m_centralLayout(new QVBoxLayout(m_centralWidget)),
    m_termStackWidget(new QStackedWidget),
    m_titlebarStyleSheet(titlebar()->styleSheet())
{
    ShortcutManager::instance();
    ShortcutManager::instance()->setMainWindow(this);
    m_shortcutManager = ShortcutManager::instance();
    ServerConfigManager::instance();
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
    // Other place (eg. create titlebar menu) will call plugin method so we should create plugins before init other parts.
    initPlugins();

    initWindow();
    initTitleBar();
    addTab(properties);
}

MainWindow::~MainWindow()
{
    //
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
    connect(termPage, &TermWidgetPage::termGetFocus, this, [ = ]() {
        CustomCommandPlugin *plugin = qobject_cast<CustomCommandPlugin *>(getPluginByName(PLUGIN_TYPE_CUSTOMCOMMAND));
        if (plugin) {
            emit plugin->doHide();
        }
    });
    connect(termPage, &TermWidgetPage::termRequestOpenCustomCommand, this, [ = ]() {
        CustomCommandPlugin *plugin = qobject_cast<CustomCommandPlugin *>(getPluginByName(PLUGIN_TYPE_CUSTOMCOMMAND));
        plugin->getCustomCommandTopPanel()->show();
    });
}

void MainWindow::closeTab(const QString &identifier)
{
    for (int i = 0, count = m_termStackWidget->count(); i < count; i++) {
        TermWidgetPage *tabPage = qobject_cast<TermWidgetPage *>(m_termStackWidget->widget(i));
        if (tabPage && tabPage->identifier() == identifier) {
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
    // apply titlebar background color
    titlebar()->setStyleSheet(QString("%1"
                                      "Dtk--Widget--DTitlebar {"
                                      "background: %2;"
                                      "}").arg(m_titlebarStyleSheet, color));
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings("blumia", "dterm");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());

    DMainWindow::closeEvent(event);
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
    QSettings settings("blumia", "dterm");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());

    setEnableBlurWindow(Settings::instance()->backgroundBlur());
}

void MainWindow::initShortcuts()
{
    m_shortcutManager->initShortcuts();

    QShortcut *focusNavUp = new QShortcut(QKeySequence("Alt+k"), this);
    connect(focusNavUp, &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) page->focusNavigation(Qt::TopEdge);
    });
    QShortcut *focusNavDown = new QShortcut(QKeySequence("Alt+j"), this);
    connect(focusNavDown, &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) page->focusNavigation(Qt::BottomEdge);
    });
    QShortcut *focusNavLeft = new QShortcut(QKeySequence("Alt+h"), this);
    connect(focusNavLeft, &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) page->focusNavigation(Qt::LeftEdge);
    });
    QShortcut *focusNavRight = new QShortcut(QKeySequence("Alt+l"), this);
    connect(focusNavRight, &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) page->focusNavigation(Qt::RightEdge);
    });

    QShortcut *splitHorizontal = new QShortcut(QKeySequence(Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_J), this);
    connect(splitHorizontal, &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) page->split(Qt::Horizontal);
    });
    QShortcut *splitVertical = new QShortcut(QKeySequence(Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_H), this);
    connect(splitVertical, &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) page->split(Qt::Vertical);
    });

    QShortcut *newTab = new QShortcut(QKeySequence(Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_T), this);
    connect(newTab, &QShortcut::activated, this, [this]() {
        this->addTab(currentTab()->createCurrentTerminalProperties(), true);
    });

    QShortcut *copyShortcut = new QShortcut(QKeySequence(Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_C), this);
    connect(copyShortcut, &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) page->copyClipboard();
    });

    QShortcut *pasteShortcut = new QShortcut(QKeySequence(Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_V), this);
    connect(pasteShortcut, &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) page->pasteClipboard();
    });

    QShortcut *zoomInShortcut = new QShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_Equal), this);
    connect(zoomInShortcut, &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) page->zoomInCurrentTierminal();
    });

    QShortcut *zoomOutShortcut = new QShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_Minus), this);
    connect(zoomOutShortcut, &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentTab();
        if (page) page->zoomOutCurrentTerminal();
    });
}

void MainWindow::initConnections()
{
    connect(Settings::instance(), &Settings::opacityChanged, this, [this](qreal opacity) {
        forAllTabPage([opacity](TermWidgetPage * tabPage) {
            tabPage->setTerminalOpacity(opacity);
        });
    });

    connect(Settings::instance(), &Settings::backgroundBlurChanged, this, [this](bool enabled) {
        setEnableBlurWindow(enabled);
    });
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

    TitleBar *titleBar = new TitleBar;
    titleBar->setTabBar(m_tabbar);
    titlebar()->setCustomWidget(titleBar);
    titlebar()->setAutoHideOnFullscreen(true);
    titlebar()->setMenu(m_menu);

    connect(m_tabbar, &DTabBar::tabCloseRequested, this, [this](int index) {
        closeTab(m_tabbar->identifier(index));
    }, Qt::QueuedConnection);

    connect(m_tabbar, &DTabBar::tabAddRequested, this, [this]() {
        addTab(currentTab()->createCurrentTerminalProperties(), true);
    }, Qt::QueuedConnection);

    connect(m_tabbar, &DTabBar::currentChanged, this, [this](int index) {
        focusTab(m_tabbar->identifier(index));
    }, Qt::QueuedConnection);

    connect(settingAction, &QAction::triggered, this, &MainWindow::showSettingDialog);
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
    QScopedPointer<DSettingsDialog> dialog(new DSettingsDialog(this));
    dialog->updateSettings(Settings::instance()->settings);
    dialog->exec();
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
