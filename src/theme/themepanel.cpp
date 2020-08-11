/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:     wangpeili <wangpeili@uniontech.com>
 *
 * Maintainer: wangpeili <wangpeili@uniontech.com>
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
#include "themepanel.h"

#include "themelistview.h"
#include "themelistmodel.h"
#include "themeitemdelegate.h"

#include <DLog>

#include <QScroller>
#include <QVBoxLayout>

ThemePanel::ThemePanel(QWidget *parent)
    : RightPanel(parent),
      m_themeView(new ThemeListView(this)),
      m_themeModel(new ThemeListModel(this))
{
    // init view.
    m_themeView->setModel(m_themeModel);
    m_themeView->setItemDelegate(new ThemeItemDelegate(m_themeView));

    setFocusProxy(m_themeView);
    QScroller::grabGesture(m_themeView, QScroller::TouchGesture);

    // init layout.
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_themeView);

    layout->setMargin(0);
    layout->setSpacing(0);

    connect(m_themeView, &ThemeListView::focusOut, this, &RightPanel::hideAnim);
    connect(m_themeView, &ThemeListView::themeChanged, this, &ThemePanel::themeChanged);
}
