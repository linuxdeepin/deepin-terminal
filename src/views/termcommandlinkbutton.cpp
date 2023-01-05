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
#include "termcommandlinkbutton.h"
#include "utils.h"

#include <DApplicationHelper>

#include <QDebug>


TermCommandLinkButton::TermCommandLinkButton(QWidget *parent)
    : DPushButton(parent)
{
    Utils::set_Object_Name(this);
    DPalette palette = DApplicationHelper::instance()->palette(this);
    palette.setColor(DPalette::ButtonText, palette.color(DPalette::TextWarning));
    this->setPalette(palette);

    this->setFlat(true);
    this->setFocusPolicy(Qt::TabFocus);
}
