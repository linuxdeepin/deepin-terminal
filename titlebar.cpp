#include "titlebar.h"

#include <DApplication>

#include <QIcon>
#include <QLabel>

DWIDGET_USE_NAMESPACE

TitleBar::TitleBar(QWidget *parent, bool isQuakeWindowStyle) : QWidget(parent), m_layout(new QHBoxLayout(this))
{
    m_layout->setContentsMargins(0, 0, 0, 0);

    if (isQuakeWindowStyle) {
        return;
    }

    if (DApplication::isDXcbPlatform()) {

        QPixmap iconPixmap = QIcon::fromTheme("deepin-terminal").pixmap(36, 36);
        QLabel *iconLabel = new QLabel(this);
        iconLabel->setFixedSize(36, 36);
        iconLabel->setPixmap(iconPixmap);


        m_layout->addSpacing(10);

        /******** Modify by n014361 wangpeili 2020-02-12: 修改居中样式***********×****/
        m_layout->addWidget(iconLabel, 0, Qt::AlignVCenter);
        /***************** Modify by n014361 End ********************×****/
        m_layout->addSpacing(10);
    }
}

TitleBar::~TitleBar()
{
}

void TitleBar::setTabBar(QWidget *widget)
{
    m_rightSpace = 70;
    /******** Modify by n014361 wangpeili 2020-02-12: 修改居中样式***********×****/
    m_layout->addWidget(widget, 0, Qt::AlignVCenter);
    /***************** Modify by n014361 End ********************×****/
    m_layout->addSpacing(m_rightSpace);
}

int TitleBar::rightSpace()
{
    return m_rightSpace;
}
