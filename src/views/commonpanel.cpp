// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "commonpanel.h"
#include "utils.h"

#include <DLog>
#ifdef DTKWIDGET_CLASS_DSizeMode
#include <DSizeMode>
#endif

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLoggingCategory>
#include <QDebug>

Q_DECLARE_LOGGING_CATEGORY(views)

CommonPanel::CommonPanel(QWidget *parent) : QFrame(parent)
{
    qCDebug(views) << "CommonPanel constructor entered";
#ifdef DTKWIDGET_CLASS_DSizeMode
    qCDebug(views) << "Branch: DTKWIDGET_CLASS_DSizeMode defined";
    // 布局模式变更时，刷新当前界面的布局，主要是按钮等高度调整等导致的效果不一致
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, [this](){
        qCDebug(views) << "Lambda: sizeModeChanged signal received";
        QRect rect = geometry();
        // 50 - 40 = 10 px
        constexpr int offset = WIN_TITLE_BAR_HEIGHT - WIN_TITLE_BAR_HEIGHT_COMPACT;
        // 普通转紧凑模式时增加高度，反之则降低高度
        rect.setHeight(rect.height() + DSizeModeHelper::element(offset, -offset));
        setGeometry(rect);

        if (layout()) {
            qCDebug(views) << "Branch: layout exists";
            layout()->invalidate();
            updateGeometry();
        }
    });
#endif
}

void CommonPanel::clearSearchInfo()
{
    qCDebug(views) << "CommonPanel::clearSearchInfo() entered";
    if (m_searchEdit) {
        qCDebug(views) << "Branch: m_searchEdit exists";
        m_searchEdit->blockSignals(true);
        m_searchEdit->clear();
        m_searchEdit->blockSignals(false);
    }
    qCDebug(views) << "CommonPanel::clearSearchInfo() finished";
}

void CommonPanel::onFocusInBackButton()
{
    qCDebug(views) << "CommonPanel::onFocusInBackButton() entered";
    if (m_rebackButton) {
        qCDebug(views) << "Branch: m_rebackButton exists";
        // 焦点进入后，选择到返回键上
        m_rebackButton->setFocus();
    }
    qCDebug(views) << "CommonPanel::onFocusInBackButton() finished";
}
