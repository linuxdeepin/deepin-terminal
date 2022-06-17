/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:     daizhengwen <daizhengwen@uniontech.com>
 *
 * Maintainer: daizhengwen <daizhengwen@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "titlebar.h"
#include "utils.h"

#include <DApplication>
#include <DIconButton>

#include <QIcon>
#include <QLabel>
#include <QDebug>
#include <QMouseEvent>

static const int VER_RESIZED_ALLOWED_OFF = 3;//允许的垂直偏移量
static const int VER_RESIZED_MIN_HEIGHT = 30;//resize的最小高度

DWIDGET_USE_NAMESPACE

TitleBar::TitleBar(QWidget *parent) : QWidget(parent), m_layout(new QHBoxLayout(this))
{
    Utils::set_Object_Name(this);
    m_layout->setObjectName("TitleBarLayout");//Add by ut001000 renfeixiang 2020-08-13
    /******** Modify by m000714 daizhengwen 2020-04-15: 标签栏和Dtk标签色保持一致****************/
//    DPalette palette = this->palette();
//    palette.setBrush(DPalette::Background, palette.color(DPalette::Base));
//    this->setPalette(palette);
    this->setBackgroundRole(DPalette::Base);
    this->setAutoFillBackground(true);
    // daizhengwen fix bug#22927 动画出的矩形框会 -50 设置标题栏为50
    this->setFixedHeight(WIN_TITLE_BAR_HEIGHT);
    /********************* Modify by m000714 daizhengwen End ************************/
    m_layout->setContentsMargins(0, 0, 0, 0);
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

void TitleBar::setVerResized(bool resized)
{
    setMouseTracking(true);
    m_verResizedEnabled = resized;
}


void TitleBar::mousePressEvent(QMouseEvent *event)
{
    QWidget *w = this->window();
    if(w) {
        int windowMouseY = this->mapTo(w, event->pos()).y();
        int windowMouseYOff = w->height() - windowMouseY;
        m_verResizedCurOff = windowMouseYOff;
    }
    QWidget::mousePressEvent(event);
}

void TitleBar::mouseMoveEvent(QMouseEvent *event)
{
    forever {
        QWidget *w = this->window();
        if(!w)
            break;
        if(!m_verResizedEnabled)
            break;
        if(!this->hasMouseTracking())
            break;
        int windowMouseY = this->mapTo(w, event->pos()).y();
        int windowMouseYOff = w->height() - windowMouseY;
        if(event->buttons() != Qt::LeftButton && windowMouseYOff < VER_RESIZED_ALLOWED_OFF) {
            this->setCursor(Qt::SizeVerCursor);
            break;
        }
        if(event->buttons() == Qt::LeftButton && m_verResizedCurOff < VER_RESIZED_ALLOWED_OFF) {
            w->resize(w->width(), qMax(VER_RESIZED_MIN_HEIGHT, windowMouseY + m_verResizedCurOff));
            break;
        }
        this->setCursor(Qt::ArrowCursor);
        break;
    }
    QWidget::mouseMoveEvent(event);
}
