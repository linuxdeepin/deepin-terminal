// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
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

CommonPanel::CommonPanel(QWidget *parent) : QFrame(parent)
{
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
