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

#ifndef TABLETABSTRACTDIALOG_H
#define TABLETABSTRACTDIALOG_H

#include <DAbstractDialog>

DWIDGET_USE_NAMESPACE

class TabletAbstractDialog : public DAbstractDialog {
    Q_OBJECT
public:
    explicit TabletAbstractDialog(QWidget *parent = nullptr);

private slots:
    void slotOnVirtualKeyboardShowHide(int windowHeight, bool isKeyboardShow);

private:
    void initConnections();

    int m_originY = -1;
};

#endif // TABLETABSTRACTDIALOG_H
