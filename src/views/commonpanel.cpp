// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "commonpanel.h"

#include <DLog>

#include <QHBoxLayout>
#include <QVBoxLayout>

CommonPanel::CommonPanel(QWidget *parent) : QFrame(parent)
{
}

void CommonPanel::clearSearchInfo()
{
    if (m_searchEdit) {
        m_searchEdit->blockSignals(true);
        m_searchEdit->clear();
        m_searchEdit->blockSignals(false);
    }
}

void CommonPanel::onFocusInBackButton()
{
    if (m_rebackButton) {
        // 焦点进入后，选择到返回键上
        m_rebackButton->setFocus();
    }
}
