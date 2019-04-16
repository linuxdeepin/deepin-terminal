#include "termwidget.h"

#include <QDebug>
#include <QVBoxLayout>

TermWidget::TermWidget(QWidget *parent)
    : QTermWidget(0, parent)
{
    setHistorySize(5000);

    // set shell program
    setShellProgram("/bin/bash");

    // config
    qDebug() << availableColorSchemes();
    setColorScheme("Linux");

    setBlinkingCursor(true);

    startShellProgram();
}

TermWidgetWrapper::TermWidgetWrapper(QWidget *parent)
    : QWidget(parent)
{
    initUI();

    connect(m_term, &QTermWidget::titleChanged, this, [this] { emit termTitleChanged(m_term->title()); });
}

void TermWidgetWrapper::initUI()
{
    m_term = new TermWidget(this);
    setFocusProxy(m_term);

    m_layout = new QVBoxLayout;
    setLayout(m_layout);

    m_layout->addWidget(m_term);
    m_layout->setContentsMargins(0, 0, 0, 0);
}
