#include "mainwindow.h"

#include "tabbar.h"

#include <QVBoxLayout>
#include <qtermwidget5/qtermwidget.h>
#include <DTitlebar>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    DMainWindow(parent),
    m_tabbar(new TabBar)
{
    this->setCentralWidget(new QWidget);
    QVBoxLayout *l = new QVBoxLayout();
    this->centralWidget()->setLayout(l);

    QTermWidget * term = new QTermWidget(0);
    term->setShellProgram("/bin/bash");
    qDebug() << term->availableColorSchemes();
    term->setColorScheme("BreezeModified");
    term->startShellProgram();
    l->addWidget(term);

    titlebar()->setCustomWidget(m_tabbar, Qt::AlignVCenter, false);
}

MainWindow::~MainWindow()
{
    //
}
