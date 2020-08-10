#include "titlebar.h"
#include "utils.h"

#include <DApplication>
#include <DIconButton>

#include <QIcon>
#include <QLabel>

DWIDGET_USE_NAMESPACE

TitleBar::TitleBar(QWidget *parent, bool isQuakeWindowStyle) : QWidget(parent), m_layout(new QHBoxLayout(this))
{
    /******** Modify by m000714 daizhengwen 2020-04-15: 标签栏和Dtk标签色保持一致****************/
//    DPalette palette = this->palette();
//    palette.setBrush(DPalette::Background, palette.color(DPalette::Base));
//    this->setPalette(palette);
    this->setBackgroundRole(DPalette::Base);
    this->setAutoFillBackground(true);
    // daizhengwen fix bug#22927 动画出的矩形框会 -50 设置标题栏为50
    this->setFixedHeight(50);
    /********************* Modify by m000714 daizhengwen End ************************/
    m_layout->setContentsMargins(0, 0, 0, 0);

    if (isQuakeWindowStyle) {
        return;
    }

    if (DApplication::isDXcbPlatform()) {

        DIconButton *iconLabel = new DIconButton(this);
        iconLabel->setIcon(QIcon::fromTheme("deepin-terminal"));
        iconLabel->setIconSize(QSize(ICONSIZE_36, ICONSIZE_36));
        iconLabel->setFocusPolicy(Qt::NoFocus);
        iconLabel->setFlat(true);

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
    /******** Modify by n014361 wangpeili 2020-02-12: 修改居中样式***********×****/
    m_layout->addWidget(widget, 0, Qt::AlignVCenter);
    /***************** Modify by n014361 End ********************×****/
}

int TitleBar::rightSpace()
{
    return m_rightSpace;
}
