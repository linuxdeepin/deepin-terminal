#include "mainwindow.h"

#include "titlebar.h"
#include "tabbar.h"
#include "themepanel.h"
#include "termwidgetpage.h"

#include <DAnchors>
#include <DTitlebar>
#include <QVBoxLayout>
#include <QDebug>
#include <QApplication>
#include <DBlurEffectWidget>
#include <QShortcut>
#include <QSettings>

DWIDGET_USE_NAMESPACE

MainWindow::MainWindow(QWidget *parent) :
    DMainWindow(parent),
    m_menu(new QMenu),
    m_tabbar(new TabBar),
    m_themePanel(new ThemePanel(this)),
    m_centralWidget(new DBlurEffectWidget(parent)),
    m_centralLayout(new QVBoxLayout(m_centralWidget)),
    m_termStackWidget(new QStackedWidget)
{
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

    DBlurEffectWidget *cwb = qobject_cast<DBlurEffectWidget*>(m_centralWidget);
    cwb->setBlendMode(DBlurEffectWidget::BlendMode::BehindWindowBlend);
    cwb->setRadius(16);

    initShortcuts();

    initWindow();
    initTitleBar();
    addTab();
}

MainWindow::~MainWindow()
{
    //
}

void MainWindow::addTab(bool activeTab)
{
    TermWidgetPage *termPage = new TermWidgetPage();
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

    QShortcut *newTab = new QShortcut(QKeySequence(Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_T), this);
    connect(newTab, &QShortcut::activated, this, [this](){
        this->addTab(true);
    });
}

void MainWindow::initTitleBar()
{
    QAction *switchThemeAction(new QAction(tr("Switch theme"), this));
    m_menu->addAction(switchThemeAction);

    TitleBar *titleBar = new TitleBar;
    titleBar->setTabBar(m_tabbar);
    titlebar()->setCustomWidget(titleBar, Qt::AlignVCenter, false);
    titlebar()->setAutoHideOnFullscreen(true);
    titlebar()->setMenu(m_menu);

    connect(m_tabbar, &DTabBar::tabCloseRequested, this, [this](int index){
        closeTab(m_tabbar->identifier(index));
    }, Qt::QueuedConnection);

    connect(m_tabbar, &DTabBar::tabAddRequested, this, [this](){
        addTab(true);
    }, Qt::QueuedConnection);

    connect(m_tabbar, &DTabBar::currentChanged, this, [this](int index){
        focusTab(m_tabbar->identifier(index));
    }, Qt::QueuedConnection);

    connect(switchThemeAction, &QAction::triggered, this, [this](){
        m_themePanel->show();
    });
}

void MainWindow::setNewTermPage(TermWidgetPage *termPage, bool activePage)
{
    m_termStackWidget->addWidget(termPage);
    if (activePage) {
        m_termStackWidget->setCurrentWidget(termPage);
    }
}
