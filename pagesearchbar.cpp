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

#include <QDebug>
#include <QGraphicsOpacityEffect>
#include <QKeyEvent>
#include <QShortcut>
#include <QApplication>

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
}

QString PageSearchBar::searchKeytxt()
{
    return m_searchEdit->text();
}

void PageSearchBar::findCancel()
{
    QWidget::hide();
    emit closeSearchBar();
}

void PageSearchBar::initFindPrevButton()
{
    m_PrevButton_Normal = new QIcon(":/images/icon/hover/arrow_left.png");
    m_PrevButton_Click = new QIcon(":/images/icon/focus/arrow_left@2x.png");

    m_findPrevButton = new QPushButton(*m_PrevButton_Normal, "");
    m_findPrevButton->setFixedSize(widgetHight, widgetHight);
    m_findPrevButton->setFocusPolicy(Qt::NoFocus);

    connect(m_findPrevButton, &QPushButton::pressed, this, [this]() {
        qDebug() << "pressed";
        m_findPrevButton->setIcon(*m_PrevButton_Click);
    });
    connect(m_findPrevButton, &QPushButton::released, this, [this]() {
        qDebug() << "released";
        m_findPrevButton->setIcon(*m_PrevButton_Normal);
        emit findPrev();
    });
}

void PageSearchBar::initFindNextButton()
{
    m_NextButton_Normal = new QIcon(":/images/icon/hover/arrow_right.png");
    m_NextButton_Click = new QIcon(":/images/icon/focus/arrow_right@2x.png");

    m_findNextButton = new QPushButton(*m_NextButton_Normal, "");
    m_findNextButton->setFixedSize(widgetHight, widgetHight);
    m_findNextButton->setFocusPolicy(Qt::NoFocus);

    connect(m_findNextButton, &QPushButton::pressed, this, [this]() {
        qDebug() << "pressed";
        m_findNextButton->setIcon(*m_NextButton_Click);
    });
    connect(m_findNextButton, &QPushButton::released, this, [this]() {
        qDebug() << "released";
        m_findNextButton->setIcon(*m_NextButton_Normal);
        emit findNext();
    });

    // 界面上输入回车就相当于直接点击下一个
    m_findNextButton->setShortcut(QKeySequence(Qt::Key_Return));
    // 由于不能绑定两个快捷键，当收到小键盘的"Enter"时候，再变身成"Return"给系统，就同时支持Enter/Return了
    QShortcut *shortcut = new QShortcut(QKeySequence(Qt::Key_Enter), this);
    connect(shortcut, &QShortcut::activated, this, [this]() {
        QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
        QApplication::sendEvent(focusWidget(), &keyPress);
    });
}

void PageSearchBar::initSearchEdit()
{
    m_searchEdit = new DSearchEdit(this);
    m_searchEdit->lineEdit()->setMinimumHeight(widgetHight);
    // 需求不让自动查找，这个接口预留
    connect(
    m_searchEdit, &DSearchEdit::textChanged, this, [this]() { emit keywordChanged(m_searchEdit->lineEdit()->text()); });
}

void PageSearchBar::setNoMatchAlert(bool isAlert)
{
    m_searchEdit->setAlert(isAlert);
}
