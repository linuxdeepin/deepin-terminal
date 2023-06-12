/*
* Copyright (C) 2022 ~ 2022 Uniontech Software Technology Co.,Ltd.
*
* Author:     Yutao Meng <mengyutao@uniontech.com>
*
* Maintainer: Yutao Meng <mengyutao@uniontech.com>
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
#pragma once

#include "listview.h"

#include <DAbstractDialog>
#include <DApplicationHelper>
#include <DLabel>
#include <DLineEdit>
#include <DFontSizeManager>
#include <DWindowCloseButton>

#include <QHBoxLayout>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE


class GroupConfigOptDlg : public DAbstractDialog
{
Q_OBJECT
public:
    GroupConfigOptDlg(const QString &groupName = QString(), QWidget *parent = nullptr);

private:
    DLabel      *m_iconLabel;
    DLabel      *m_titleLabel;
    DLineEdit   *m_groupNameEdit;
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_headLayout;
    DWindowCloseButton *m_closeButton;
    ListView           *m_serverList;
};
