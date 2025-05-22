// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "encodepanel.h"

#include "encodelistview.h"
#include "encodelistmodel.h"
#include "settings.h"

#include <DLog>
#include <QScroller>
#include <QVBoxLayout>

Q_DECLARE_LOGGING_CATEGORY(encodeplugin)

EncodePanel::EncodePanel(QWidget *parent)
    : RightPanel(parent), m_encodeView(new EncodeListView(this))
{
    qCDebug(encodeplugin) << "EncodePanel constructor enter";
    /******** Add by ut001000 renfeixiang 2020-08-14:增加 Begin***************/
    Utils::set_Object_Name(this);
    m_encodeView->setObjectName("EncodeListView");
    /******** Add by ut001000 renfeixiang 2020-08-14:增加 End***************/
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    setFocusProxy(m_encodeView);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addWidget(m_encodeView);

    // init layout.
    QVBoxLayout *layout = new QVBoxLayout(this);
    /******** Modify by nt001000 renfeixiang 2020-05-16:解决Alt+F2显示Encode时，高度变长的问题 Begin***************/
    /*layout->addSpacing(10);增加的spacing会影响m_encodeView的高度*/
    layout->addLayout(hLayout);
    layout->addStretch();
    layout->setContentsMargins(0, 0, 0, 0);//增加的Margin会影响m_encodeView的高度
    layout->setSpacing(0);
    /******** Modify by nt001000 renfeixiang 2020-05-16:解决Alt+F2显示Encode时，高度变长的问题 Begin***************/

    connect(m_encodeView, &EncodeListView::focusOut, this, &RightPanel::hideAnim);
    qCDebug(encodeplugin) << "EncodePanel constructor exit";
}

void EncodePanel::show()
{
    qCDebug(encodeplugin) << "EncodePanel show enter";
    this->showAnim();

    //解决Alt+F2显示Encode时，高度变长的问题 每次显示时，设置固定高度 Begin
    m_encodeView->setFixedHeight(size().height());
    //解决Alt+F2显示Encode时，高度变长的问题 End
    qCDebug(encodeplugin) << "EncodePanel show exit";
}

void EncodePanel::updateEncode(QString encode)
{
    qCDebug(encodeplugin) << "updateEncode enter, encode:" << encode;
    m_encodeView->checkEncode(encode);
    qCDebug(encodeplugin) << "updateEncode exit";
}

