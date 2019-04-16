#include "titlebar.h"

#include <QIcon>
#include <QLabel>

TitleBar::TitleBar(QWidget *parent)
    : QWidget(parent),
      m_layout(new QHBoxLayout(this))
{
    m_layout->setContentsMargins(0, 0, 0, 0);

    QPixmap iconPixmap = QIcon::fromTheme("deepin-terminal").pixmap(24, 24);
    QLabel *iconLabel = new QLabel(this);
    iconLabel->setPixmap(iconPixmap);
    iconLabel->setFixedSize(24, 40);

    m_layout->addSpacing(10);
    m_layout->addWidget(iconLabel, 0, Qt::AlignTop);
    m_layout->addSpacing(10);
}

TitleBar::~TitleBar()
{

}

void TitleBar::setTabBar(QWidget *widget)
{
    m_layout->addWidget(widget, 0, Qt::AlignTop);
    m_layout->addSpacing(70);
}
