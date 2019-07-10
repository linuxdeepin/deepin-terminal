#include "mainwindow.h"

#include "titlebar.h"
#include "tabbar.h"
#include "themepanel.h"
#include "settings.h"
#include "termwidgetpage.h"
#include "termproperties.h"

#include <DAnchors>
#include <DTitlebar>
#include <QVBoxLayout>
#include <QDebug>
#include <QApplication>
#include <QShortcut>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>

DWIDGET_USE_NAMESPACE

MainWindow::MainWindow(TermProperties properties, QWidget *parent) :
    DMainWindow(parent),
    m_menu(new QMenu),
    m_tabbar(new TabBar),
    m_themePanel(new ThemePanel(this)),
    m_centralWidget(new QWidget(this)),
    m_centralLayout(new QVBoxLayout(m_centralWidget)),
    m_termStackWidget(new QStackedWidget)
{
    setAttribute(Qt::WA_TranslucentBackground);

    setMinimumSize(450, 250);
    setCentralWidget(m_centralWidget);
    setWindowIcon(QIcon::fromTheme("deepin-terminal"));

    // Init layout
    m_centralLayout->setMargin(0);
    m_centralLayout->setSpacing(0);

    m_centralLayout->addWidget(m_termStackWidget);

    // Init theme panel.
    DAnchorsBase::setAnchor(m_themePanel, Qt::AnchorTop, m_centralWidget, Qt::AnchorTop);
    DAnchorsBase::setAnchor(m_themePanel, Qt::AnchorBottom, m_centralWidget, Qt::AnchorBottom);
    DAnchorsBase::setAnchor(m_themePanel, Qt::AnchorRight, m_centralWidget, Qt::AnchorRight);

    initConnections();
    initShortcuts();
    initCustomCommands();

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
    connect(termPage, &TermWidgetPage::lastTermClosed, this, &MainWindow::closeTab);
}

void MainWindow::closeTab(const QString &identifier)
{
    for (int i = 0, count = m_termStackWidget->count(); i < count; i++) {
        TermWidgetPage *tabPage = qobject_cast<TermWidgetPage*>(m_termStackWidget->widget(i));
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
        TermWidgetPage *tabPage = qobject_cast<TermWidgetPage*>(m_termStackWidget->widget(i));
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

void MainWindow::closeEvent(QCloseEvent *event) {
    QSettings settings("blumia", "dterm");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());

    DMainWindow::closeEvent(event);
}

void MainWindow::onTermTitleChanged(QString title)
{
    TermWidgetPage *tabPage = qobject_cast<TermWidgetPage*>(sender());
    const bool customName = tabPage->property("TAB_CUSTOM_NAME_PROPERTY").toBool();
    if (!customName) {
        m_tabbar->setTabText(tabPage->identifier(), title);
    }
}

void MainWindow::onTabTitleChanged(QString title)
{
    TermWidgetPage *tabPage = qobject_cast<TermWidgetPage*>(sender());
    m_tabbar->setTabText(tabPage->identifier(), title);
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
    QShortcut *focusNavUp = new QShortcut(QKeySequence("Alt+k"), this);
    connect(focusNavUp, &QShortcut::activated, this, [this](){
        TermWidgetPage *page = currentTab();
        if (page) page->focusNavigation(Up);
    });
    QShortcut *focusNavDown = new QShortcut(QKeySequence("Alt+j"), this);
    connect(focusNavDown, &QShortcut::activated, this, [this](){
        TermWidgetPage *page = currentTab();
        if (page) page->focusNavigation(Down);
    });
    QShortcut *focusNavLeft = new QShortcut(QKeySequence("Alt+h"), this);
    connect(focusNavLeft, &QShortcut::activated, this, [this](){
        TermWidgetPage *page = currentTab();
        if (page) page->focusNavigation(Left);
    });
    QShortcut *focusNavRight = new QShortcut(QKeySequence("Alt+l"), this);
    connect(focusNavRight, &QShortcut::activated, this, [this](){
        TermWidgetPage *page = currentTab();
        if (page) page->focusNavigation(Right);
    });

    QShortcut *splitHorizontal = new QShortcut(QKeySequence(Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_J), this);
    connect(splitHorizontal, &QShortcut::activated, this, [this](){
        TermWidgetPage *page = currentTab();
        if (page) page->split(Qt::Horizontal);
    });
    QShortcut *splitVertical = new QShortcut(QKeySequence(Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_H), this);
    connect(splitVertical, &QShortcut::activated, this, [this](){
        TermWidgetPage *page = currentTab();
        if (page) page->split(Qt::Vertical);
    });

    QShortcut *newTab = new QShortcut(QKeySequence(Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_T), this);
    connect(newTab, &QShortcut::activated, this, [this](){
        this->addTab(currentTab()->createCurrentTerminalProperties(), true);
    });

    QShortcut *copyShortcut = new QShortcut(QKeySequence(Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_C), this);
    connect(copyShortcut, &QShortcut::activated, this, [this](){
        TermWidgetPage *page = currentTab();
        if (page) page->copyClipboard();
    });

    QShortcut *pasteShortcut = new QShortcut(QKeySequence(Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_V), this);
    connect(pasteShortcut, &QShortcut::activated, this, [this](){
        TermWidgetPage *page = currentTab();
        if (page) page->pasteClipboard();
    });

    QShortcut *zoomInShortcut = new QShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_Equal), this);
    connect(zoomInShortcut, &QShortcut::activated, this, [this](){
        TermWidgetPage *page = currentTab();
        if (page) page->zoomInCurrentTierminal();
    });

    QShortcut *zoomOutShortcut = new QShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_Minus), this);
    connect(zoomOutShortcut, &QShortcut::activated, this, [this](){
        TermWidgetPage *page = currentTab();
        if (page) page->zoomOutCurrentTerminal();
    });
}

void MainWindow::initConnections()
{
    connect(m_themePanel, &ThemePanel::themeChanged, this, [this](const QString themeName) {
        forAllTabPage([themeName](TermWidgetPage * tabPage) {
            tabPage->setColorScheme(themeName);
        });
        Settings::instance()->setColorScheme(themeName);
    });

    connect(Settings::instance(), &Settings::opacityChanged, this, [this](qreal opacity) {
        forAllTabPage([opacity](TermWidgetPage * tabPage) {
            tabPage->setTerminalOpacity(opacity);
        });
    });

    connect(Settings::instance(), &Settings::backgroundBlurChanged, this, [this](bool enabled) {
        setEnableBlurWindow(enabled);
    });
}

void MainWindow::initCustomCommands()
{
    QDir customCommandBasePath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    if (!customCommandBasePath.exists()) {
        return;
    }

    QString customCommandConfigFilePath(customCommandBasePath.filePath("command-config.conf"));
    if (!QFile::exists(customCommandConfigFilePath)) {
        return;
    }

    QSettings commandsSettings(customCommandConfigFilePath, QSettings::IniFormat);
    QStringList commandGroups = commandsSettings.childGroups();
    for (const QString &commandName : commandGroups) {
        commandsSettings.beginGroup(commandName);
        if (!commandsSettings.contains("Command")) continue;
        QAction * action = new QAction(commandName, this);
        action->setData(commandsSettings.value("Command").toString()); // make sure it is a QString
        if (commandsSettings.contains("Shortcut")) {
            QVariant shortcutVariant = commandsSettings.value("Shortcut");
            if (shortcutVariant.type() == QVariant::KeySequence) {
                action->setShortcut(shortcutVariant.convert(QMetaType::QKeySequence));
            } else if (shortcutVariant.type() == QVariant::String) {
                // to make it compatible to deepin-terminal config file.
                QString shortcutStr = shortcutVariant.toString().remove(QChar(' '));
                action->setShortcut(QKeySequence(shortcutStr));
            }
        }
        this->addAction(action);
        connect(action, &QAction::triggered, this, [this, action](){
            QString command = action->data().toString();
            if (!command.endsWith('\n')) {
                command.append('\n');
            }
            this->currentTab()->sendTextToCurrentTerm(command);
        });
    }
}

void MainWindow::initTitleBar()
{
    QAction *switchThemeAction(new QAction(tr("Switch &theme"), this));
    m_menu->addAction(switchThemeAction);
    QAction *settingAction(new QAction(tr("&Settings"), this));

    m_menu->addAction(settingAction);

    TitleBar *titleBar = new TitleBar;
    titleBar->setTabBar(m_tabbar);
    titlebar()->setCustomWidget(titleBar, Qt::AlignVCenter, false);
    titlebar()->setAutoHideOnFullscreen(true);
    titlebar()->setMenu(m_menu);

    connect(m_tabbar, &DTabBar::tabCloseRequested, this, [this](int index){
        closeTab(m_tabbar->identifier(index));
    }, Qt::QueuedConnection);

    connect(m_tabbar, &DTabBar::tabAddRequested, this, [this](){
        addTab(currentTab()->createCurrentTerminalProperties(), true);
    }, Qt::QueuedConnection);

    connect(m_tabbar, &DTabBar::currentChanged, this, [this](int index){
        focusTab(m_tabbar->identifier(index));
    }, Qt::QueuedConnection);

    connect(switchThemeAction, &QAction::triggered, this, [this](){
        m_themePanel->show();
    });
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

void MainWindow::forAllTabPage(const std::function<void(TermWidgetPage*)> &func)
{
    for (int i = 0, count = m_termStackWidget->count(); i < count; i++) {
        TermWidgetPage *tabPage = qobject_cast<TermWidgetPage*>(m_termStackWidget->widget(i));
        if (tabPage) {
            func(tabPage);
        }
    }
}
