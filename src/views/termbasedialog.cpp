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
#include "termbasedialog.h"

#include <QLabel>
#include <DApplicationHelper>

static void palrtteTransparency(QWidget *widget, qint8 alphaFloat)
{
    QPalette palette = widget->palette();
    QColor color = DGuiApplicationHelper::adjustColor(palette.color(QPalette::BrightText), 0, 0, 0, 0, 0, 0, alphaFloat);
    palette.setColor(QPalette::WindowText, color);
    widget->setPalette(palette);
}

//fix bug 23481 主菜单切换主题，弹框字体颜色没有随主题及时变换
void QWidget::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)

    if (strcmp(this->metaObject()->className(), "Dtk::Widget::DDialog") != 0)
    {
        return;
    }

    QLabel *titleLabel = this->findChild<QLabel *>("TitleLabel");
    QLabel *messageLabel = this->findChild<QLabel *>("MessageLabel");

    if (titleLabel)
    {
        palrtteTransparency(titleLabel, -10);
    }

    if (messageLabel)
    {
        palrtteTransparency(messageLabel, -30);
    }
}
