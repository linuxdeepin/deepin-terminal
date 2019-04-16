#include "mainwindow.h"

#include "titlebar.h"
#include "tabbar.h"
#include "termwidgetpage.h"

#include <QVBoxLayout>
#include <DTitlebar>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    DMainWindow(parent),
    m_tabbar(new TabBar),
    m_centralWidget(new QWidget),
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

    initTitleBar();

    addTab();
}

MainWindow::~MainWindow()
{
    //
}

void MainWindow::addTab(bool activeTab)
{
    Q_UNUSED(activeTab);
    TermWidgetPage *termPage = new TermWidgetPage();
    setNewTermPage(termPage, activeTab);
    int index = m_tabbar->addTab(termPage->identifier(), "New Terminal Tab");
    if (activeTab) {
        m_tabbar->setCurrentIndex(index);
    }
    connect(termPage, &TermWidgetPage::termTitleChanged, this, &MainWindow::onTermTitleChanged);
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
}

void MainWindow::focusTab(const QString &identifier)
{
    for (int i = 0, count = m_termStackWidget->count(); i < count; i++) {
        TermWidgetPage *tabPage = qobject_cast<TermWidgetPage*>(m_termStackWidget->widget(i));
        if (tabPage && tabPage->identifier() == identifier) {
            m_termStackWidget->setCurrentWidget(tabPage);
            break;
        }
    }
}

void MainWindow::onTermTitleChanged(QString title)
{
    TermWidgetPage *tabPage = qobject_cast<TermWidgetPage*>(sender());
    const bool customName = tabPage->property("TAB_CUSTOM_NAME_PROPERTY").toBool();
    if (!customName) {
        m_tabbar->setTabText(tabPage->identifier(), title);
    }
}

void MainWindow::initTitleBar()
{
    TitleBar *titleBar = new TitleBar;
    titleBar->setTabBar(m_tabbar);
    titlebar()->setCustomWidget(titleBar, Qt::AlignVCenter, false);

    connect(m_tabbar, &DTabBar::tabCloseRequested, this, [this](int index){
        closeTab(m_tabbar->identifier(index));
    }, Qt::QueuedConnection);

    connect(m_tabbar, &DTabBar::tabAddRequested, this, [this](){
        addTab(true);
    }, Qt::QueuedConnection);

    connect(m_tabbar, &DTabBar::currentChanged, this, [this](int index){
        focusTab(m_tabbar->identifier(index));
    }, Qt::QueuedConnection);
}

void MainWindow::setNewTermPage(TermWidgetPage *termPage, bool activePage)
{
    m_termStackWidget->addWidget(termPage);
    if (activePage) {
        m_termStackWidget->setCurrentWidget(termPage);
    }
}
