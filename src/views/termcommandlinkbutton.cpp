// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "termcommandlinkbutton.h"
#include "utils.h"

#include <DPaletteHelper>

#include <QDebug>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(views)

TermCommandLinkButton::TermCommandLinkButton(QWidget *parent)
    : DPushButton(parent)
{
    qCDebug(views) << "TermCommandLinkButton constructor enter";
    Utils::set_Object_Name(this);
    
    DPalette palette = DPaletteHelper::instance()->palette(this);
    qCDebug(views) << "Setting button text color to warning color";
    palette.setColor(DPalette::ButtonText, palette.color(DPalette::TextWarning));
    this->setPalette(palette);

    qCDebug(views) << "Configuring button properties";
    this->setFlat(true);
    this->setFocusPolicy(Qt::TabFocus);
    qCDebug(views) << "TermCommandLinkButton constructor exit";
}
