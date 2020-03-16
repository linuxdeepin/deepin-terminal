/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2011 ~ 2018 Deepin, Inc.
 *
 * Author:     Wang Yong <wangpeili@deepin.com>
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

#ifndef NEWDSPINBOX_H
#define NEWDSPINBOX_H

#include <DLineEdit>
#include <DIconButton>
#include <QHBoxLayout>
#include <QRegExpValidator>

DWIDGET_USE_NAMESPACE

class NewDspinBox : public QWidget
{
    Q_OBJECT
public:
    explicit NewDspinBox(QWidget *parent = 0);
    void setValue(int val);
    void setMaximum(int val);
    void setMinimum(int val);

signals:
    void valueChanged(int value);
private:
    void correctValue();

private:

    DLineEdit * m_DLineEdit= nullptr;
    DIconButton *m_DIconBtnAdd = nullptr;
    DIconButton *m_DIconBtnSubtract = nullptr;

    QIntValidator * m_QIntValidator = nullptr;
    int m_MaxValue = 100;
    int m_MinValue = 0;
};



#endif
