/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2011 ~ 2018 Deepin, Inc.
 *
 * Author:     Wang Yong <wangyong@deepin.com>
 * Maintainer: Rekols    <rekols@foxmail.com>
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

#ifndef PAGESEARCHBAR_H
#define PAGESEARCHBAR_H

#include <DIconButton>
#include <DApplicationHelper>
#include <DFloatingWidget>
#include <DPalette>
#include <DSearchEdit>
#include <DPushButton>

#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE

class PageSearchBar : public DFloatingWidget
{
    Q_OBJECT
public:
    PageSearchBar(QWidget *parent = 0);

    bool isFocus();
    void focus();
    void setNoMatchAlert(bool isAlert);
    QString searchKeytxt();

signals:
    void findNext();
    void findPrev();
    void keywordChanged(QString keyword);
    void closeSearchBar();

public slots:
    void findCancel();

private:
    void initFindPrevButton();
    void initFindNextButton();
    void initSearchEdit();

    DIconButton *m_findNextButton = nullptr;
    DIconButton *m_findPrevButton = nullptr;
    DSearchEdit *m_searchEdit = nullptr;

    const int barHight = 50;
    const int barWidth = 382;
    const int layoutMargins = 7;
    const int widgetHight = 36;
    const int widgetSpace = 10;

    const int iconHight = 6;
    const int iconWidth = 12;

    const qreal opacity = 0.9;
};

#endif
