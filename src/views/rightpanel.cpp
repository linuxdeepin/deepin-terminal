// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "rightpanel.h"
#include "utils.h"
#include "mainwindow.h"

//dtk
#include <DAnchors>
#include <DTitlebar>

#ifdef DTKWIDGET_CLASS_DSizeMode
#include <DSizeMode>
#endif

//qt
#include <QPropertyAnimation>

DWIDGET_USE_NAMESPACE

RightPanel::RightPanel(QWidget *parent) : QWidget(parent)
{
    // hide by default.
    QWidget::hide();
    // Init theme panel.
    setFixedWidth(240 + 2);

#ifdef DTKWIDGET_CLASS_DSizeMode
    // 布局模式变更时，刷新当前界面的布局，主要是按钮等高度调整等导致的效果不一致
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, [this](){
        QRect rect = geometry();
        // 50 - 40 = 10 px
        constexpr int offset = WIN_TITLE_BAR_HEIGHT - WIN_TITLE_BAR_HEIGHT_COMPACT;
        // 普通转紧凑模式时增加高度，反之则降低高度
        rect.setHeight(rect.height() + DSizeModeHelper::element(offset, -offset));
        setGeometry(rect);

        if (layout()) {
            layout()->invalidate();
            updateGeometry();
        }
    });
#endif
}

void RightPanel::showAnim()
{
    QWidget::show();
    QWidget::raise();

    QRect rect = geometry();
    QRect windowRect = window()->geometry();
    QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry",this);
    animation->setDuration(250);
    animation->setEasingCurve(QEasingCurve::OutQuad);
    /***mod begin by ut001121 zhangmeng 20200918 修复BUG48374 全屏下插件被截断的问题***/
    int panelHeight = windowRect.height();
    MainWindow *w = Utils::getMainWindow(this);
    if (w && w->titlebar()->isVisible()) {
#ifdef DTKWIDGET_CLASS_DSizeMode
        panelHeight -= DSizeModeHelper::element(WIN_TITLE_BAR_HEIGHT_COMPACT, WIN_TITLE_BAR_HEIGHT);
#else
        panelHeight -= WIN_TITLE_BAR_HEIGHT;
#endif
    }

    animation->setStartValue(QRect(windowRect.width(), rect.y(), rect.width(), panelHeight));
    animation->setEndValue(QRect(windowRect.width() - rect.width(), rect.y(), rect.width(), panelHeight));
    /***mod end by ut001121***/
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void RightPanel::hideAnim()
{
    // 隐藏状态不处理
    if (!isVisible())
        return;

    QRect rect = geometry();
    QRect windowRect = window()->geometry();
    QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry", this);
    animation->setDuration(250);
    animation->setEasingCurve(QEasingCurve::OutQuad);
    /***mod begin by ut001121 zhangmeng 20200918 修复BUG48374 全屏下插件被截断的问题***/
    int panelHeight = windowRect.height();
    MainWindow *w = Utils::getMainWindow(this);
    if (w && w->titlebar()->isVisible()) {
#ifdef DTKWIDGET_CLASS_DSizeMode
        panelHeight -= DSizeModeHelper::element(WIN_TITLE_BAR_HEIGHT_COMPACT, WIN_TITLE_BAR_HEIGHT);
#else
        panelHeight -= WIN_TITLE_BAR_HEIGHT;
#endif
    }

    animation->setStartValue(QRect(windowRect.width() - rect.width(), rect.y(), rect.width(), panelHeight));
    animation->setEndValue(QRect(windowRect.width(), rect.y(), rect.width(), panelHeight));
    /***mod end by ut001121***/

    /***mod begin by ut001121 zhangmeng 20200924 修复BUG49378***/
    //结束处理
    connect(animation, &QPropertyAnimation::finished, this, [ = ] {
        //启用面板
        setEnabled(true);
        //隐藏面板
        hide();
    });

    //禁用面板
    setEnabled(false);

    //开始动画
    animation->start(QAbstractAnimation::DeleteWhenStopped);
    /***mod end by ut001121***/
}

void RightPanel::hideEvent(QHideEvent *event)
{
    /***add begin by ut001121 zhangmeng 20200801 解决终端插件被隐藏时焦点回退到工作区的问题***/
    if (QApplication::focusWidget()) {
        // 焦点控件的全局坐标
        QPoint focusPoint = QApplication::focusWidget()->mapToGlobal(QPoint(0, 0));
        // 焦点相对当前控件坐标
        focusPoint = mapFromGlobal(focusPoint);

        // 判断是否包含坐标
        if (rect().contains(focusPoint)) {
            MainWindow *w = Utils::getMainWindow(this);
            if(w)
                w->focusCurrentPage();
        }
    }
    /***add end by ut001121***/

    QWidget::hideEvent(event);
}
