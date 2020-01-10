#include "titlebar.h"

#include <DApplication>

#include <QIcon>
#include <QLabel>

DWIDGET_USE_NAMESPACE

TitleBar::TitleBar(QWidget *parent, bool isQuakeWindowStyle)
    : QWidget(parent),
      m_layout(new QHBoxLayout(this))
{
    m_layout->setContentsMargins(0, 0, 0, 0);

    if (isQuakeWindowStyle) {
        return;
    }

    if (DApplication::isDXcbPlatform()) {

        QPixmap iconPixmap = QIcon::fromTheme("deepin-terminal").pixmap(24, 24);
        QLabel *iconLabel = new QLabel(this);
        iconLabel->setPixmap(iconPixmap);
        iconLabel->setFixedSize(24, 40);

        m_layout->addSpacing(10);
        m_layout->addWidget(iconLabel, 0, Qt::AlignTop);
        m_layout->addSpacing(10);
    }
}

TitleBar::~TitleBar()
{
}

void TitleBar::setTabBar(QWidget *widget)
{
    m_rightSpace = 70;
    m_layout->addWidget(widget, 0, Qt::AlignTop);
    m_layout->addSpacing(m_rightSpace);
}

int TitleBar::rightSpace()
{
    return m_rightSpace;
}
