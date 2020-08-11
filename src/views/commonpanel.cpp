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

/*******************************************************************************
 1. @函数:    focusInBackButton
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-22
 4. @说明:    回车后，焦点进入返回键
*******************************************************************************/
void CommonPanel::onFocusInBackButton()
{
    qDebug() << __FUNCTION__;
    if (m_rebackButton) {
        // 焦点进入后，选择到返回键上
        m_rebackButton->setFocus();
    }
}
