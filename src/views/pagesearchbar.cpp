/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2011 ~ 2018 Deepin, Inc.
 *
 * Author:     Wang Peili <wangpeili@deepin.com>
 * Maintainer: Rekols     <rekols@foxmail.com>
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

#include "pagesearchbar.h"
#include "utils.h"

#include <DLog>
#include <QToolButton>

#include <QGraphicsOpacityEffect>
#include <QKeyEvent>
#include <QShortcut>
#include <QApplication>
#include <QAction>

PageSearchBar::PageSearchBar(QWidget *parent) : DFloatingWidget(parent)
{
    // Init
    hide();
    setFixedSize(barWidth, barHight + 12);
    // 设置窗体透明度的，需求为100%
    QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect;
    setGraphicsEffect(opacityEffect);
    opacityEffect->setOpacity(opacity);

    initSearchEdit();
    initFindNextButton();
    initFindPrevButton();

    // Init layout and widgets.
    QHBoxLayout *m_layout = new QHBoxLayout();
    m_layout->setSpacing(widgetSpace);
    m_layout->setContentsMargins(layoutMargins, layoutMargins, layoutMargins, layoutMargins);
    m_layout->addWidget(m_searchEdit);
    m_layout->addWidget(m_findPrevButton);
    m_layout->addWidget(m_findNextButton);
    setLayout(m_layout);

    // Esc隐藏
    QShortcut *shortcut = new QShortcut(QKeySequence::Cancel, this);
    connect(shortcut, &QShortcut::activated, this, [this]() { findCancel(); });
}

bool PageSearchBar::isFocus()
{
    return m_searchEdit->lineEdit()->hasFocus();
}

void PageSearchBar::focus()
{
    m_searchEdit->lineEdit()->setFocus();
    m_searchEdit->lineEdit()->selectAll();
    //　焦点一进入以后，就设置文字和图标，用于失去焦点后显示
    recoveryHoldContent();
}

QString PageSearchBar::searchKeytxt()
{
    return m_searchEdit->text();
}

void PageSearchBar::saveOldHoldContent()
{
    m_originalPlaceHolder = m_searchEdit->placeHolder();
}

//查找DSearchEdit中的DIconButton
DIconButton *findIconBtn(DSearchEdit *searchEdit)
{
    QWidget *iconWidget = searchEdit->findChild<QWidget *>("iconWidget");
    if (iconWidget) {
        DIconButton *iconBtn = iconWidget->findChild<DIconButton *>();
        return iconBtn;
    } else {
        return searchEdit->findChild<DIconButton *>();
    }
}

void PageSearchBar::clearHoldContent()
{
    // 置空内容
    m_searchEdit->setPlaceHolder("");
    DIconButton *iconBtn = findIconBtn(m_searchEdit);
    if (iconBtn) {
        iconBtn->setIcon(QIcon(""));
    }
}

void PageSearchBar::recoveryHoldContent()
{
    // 还原文本
    m_searchEdit->setPlaceHolder(m_originalPlaceHolder);
    DIconButton *iconBtn = findIconBtn(m_searchEdit);
    if (iconBtn) {
        // 还原图标
        iconBtn->setIcon(DStyle::SP_IndicatorSearch);
    }
}

void PageSearchBar::findCancel()
{
    QWidget::hide();
    emit closeSearchBar();
}

void PageSearchBar::initFindPrevButton()
{
    m_findPrevButton = new DIconButton(QStyle::SP_ArrowUp);
    m_findPrevButton->setFixedSize(widgetHight, widgetHight);
    m_findPrevButton->setFocusPolicy(Qt::NoFocus);

    connect(m_findPrevButton, &QAbstractButton::clicked, this, [this]() {
        if (!m_searchEdit->lineEdit()->text().isEmpty()) {
            emit findPrev();
        }
    });

    QAction *action = new QAction(m_findPrevButton);
    QList<QKeySequence> lstShortcut;
    lstShortcut << QKeySequence("Shift+Enter") << QKeySequence("Shift+Return");
    action->setShortcuts(lstShortcut);
    m_findPrevButton->addAction(action);
    connect(action, &QAction::triggered, this, [this]() { m_findPrevButton->animateClick(80); });
}

void PageSearchBar::initFindNextButton()
{
    m_findNextButton = new DIconButton(QStyle::SP_ArrowDown);
    m_findNextButton->setFixedSize(widgetHight, widgetHight);
    m_findNextButton->setFocusPolicy(Qt::NoFocus);

    connect(m_findNextButton, &QAbstractButton::clicked, this, [this]() {
        if (!m_searchEdit->lineEdit()->text().isEmpty()) {
            emit findNext();
        }
    });

    // 界面上输入回车就相当于直接点击下一个:Key_Enter OR Key_Return
    // 控件本身不支持设置多个快捷键
    QAction *action = new QAction(m_findNextButton);
    QList<QKeySequence> lstShortcut;
    lstShortcut << QKeySequence(Qt::Key_Enter) << QKeySequence(Qt::Key_Return);
    action->setShortcuts(lstShortcut);
    m_findNextButton->addAction(action);
    connect(action, &QAction::triggered, this, [this]() { m_findNextButton->animateClick(80); });
}

void PageSearchBar::initSearchEdit()
{
    m_searchEdit = new DSearchEdit(this);
    m_searchEdit->lineEdit()->setMinimumHeight(widgetHight);

    //　保留原文字，图标
    saveOldHoldContent();
    // 置空
    clearHoldContent();

    // 把ＤＴＫ好容易改造的功能，还原了．．．．．．．．
    // 把那个＂Ｘ＂控件功能还原成仅为清空文本
    QList<QToolButton *> list = m_searchEdit->lineEdit()->findChildren<QToolButton *>();
    QAction *clearAction = m_searchEdit->lineEdit()->findChild<QAction *>(QLatin1String("_q_qlineeditclearaction"));
    for (int i = 0; i < list.count(); i++) {
        if (list.at(i)->defaultAction() == clearAction) {
            QToolButton *clearBtn = list.at(i);
            //屏蔽lineedit清除按钮的槽函数,_q_clearFocus()获得有效的判断条件
            clearBtn->disconnect(SIGNAL(clicked()));
            connect(clearBtn, &QToolButton::clicked, [this]() {
                m_searchEdit->lineEdit()->setText("");
            });
        }
    }

    // 需求不让自动查找，这个接口预留
    connect(m_searchEdit, &DSearchEdit::textChanged, this, [this]() {
        emit keywordChanged(m_searchEdit->lineEdit()->text());
    });
}

void PageSearchBar::setNoMatchAlert(bool isAlert)
{
    m_searchEdit->setAlert(isAlert);
}
