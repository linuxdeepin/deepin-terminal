// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "termcommandlinkbutton.h"
#include "utils.h"

#include <DApplicationHelper>
#include <DPaletteHelper>

#include <QDebug>


TermCommandLinkButton::TermCommandLinkButton(QWidget *parent)
    : DPushButton(parent)
{
    Utils::set_Object_Name(this);
    DPalette palette = DPaletteHelper::instance()->palette(this);
    palette.setColor(DPalette::ButtonText, palette.color(DPalette::TextWarning));
    this->setPalette(palette);

    this->setFlat(true);
    this->setFocusPolicy(Qt::TabFocus);
}
