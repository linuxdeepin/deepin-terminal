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

    TitleBar *titleBar = new TitleBar;
    titleBar->setTabBar(m_tabbar);
    titlebar()->setCustomWidget(titleBar, Qt::AlignVCenter, false);

    addTab();
}

MainWindow::~MainWindow()
{
    //
}

void MainWindow::addTab(bool activeTab)
{
    TermWidgetPage *termPage = new TermWidgetPage();
    setNewTermPage(termPage);
    m_tabbar->addTab("", "Placeholder");
    connect(termPage, &TermWidgetPage::termTitleChanged, this, &MainWindow::onTermTitleChanged);
}

void MainWindow::onTermTitleChanged(QString title)
{
    qDebug() << "workssssssssssssssss";
    TermWidgetPage *tabPage = qobject_cast<TermWidgetPage*>(sender());
    const bool customName = tabPage->property("TAB_CUSTOM_NAME_PROPERTY").toBool();
    if (!customName) {
        const QString identifier = tabPage->property("TAB_IDENTIFIER_PROPERTY").toString();
        m_tabbar->setTabText(identifier, title);
    }
}

void MainWindow::setNewTermPage(TermWidgetPage *termPage)
{
//    TermWidgetPage * termpage = new TermWidgetPage(this);
//    l->addWidget(termpage);
    m_termStackWidget->addWidget(termPage);
    m_termStackWidget->setCurrentWidget(termPage);
}
