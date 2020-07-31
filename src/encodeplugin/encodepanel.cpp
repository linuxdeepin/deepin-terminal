/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:zhangmeng@uniontech.com
 *
 * Maintainer:编码插件列表面板
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

#include "encodepanel.h"

#include "encodelistview.h"
#include "encodelistmodel.h"
#include "settings.h"

#include <DLog>
#include <QScroller>
#include <QVBoxLayout>

EncodePanel::EncodePanel(QWidget *parent)
    : RightPanel(parent), m_encodeView(new EncodeListView(this))
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    setFocusProxy(m_encodeView);

    /********************* Modify by m000714 daizhengwen End ************************/
    QScroller::grabGesture(m_encodeView, QScroller::TouchGesture);

    QHBoxLayout *hLayout = new QHBoxLayout();
    /*hLayout->addSpacing(10);*/
    hLayout->addWidget(m_encodeView);
    /*hLayout->addSpacing(10);*/

    // init layout.
    QVBoxLayout *layout = new QVBoxLayout(this);
    /******** Modify by nt001000 renfeixiang 2020-05-16:解决Alt+F2显示Encode时，高度变长的问题 Begin***************/
    /*layout->addSpacing(10);增加的spacing会影响m_encodeView的高度*/
    layout->addLayout(hLayout);
    layout->addStretch();
    layout->setMargin(0);//增加的Margin会影响m_encodeView的高度
    layout->setSpacing(0);
    /******** Modify by nt001000 renfeixiang 2020-05-16:解决Alt+F2显示Encode时，高度变长的问题 Begin***************/

    connect(m_encodeView, &EncodeListView::focusOut, this, &RightPanel::hideAnim);
}

/*******************************************************************************
 1. @函数:    show
 2. @作者:    nt001000 renfeixiang
 3. @日期:    2020-05-16
 4. @说明:    处理显示(设置m_encodeView的大小)
*******************************************************************************/
void EncodePanel::show()
{
    RightPanel::show();
    qDebug() << "EncodePanelEncodePanelshow" << size().height();

    //解决Alt+F2显示Encode时，高度变长的问题 每次显示时，设置固定高度 Begin
    m_encodeView->setFixedHeight(size().height());
    //解决Alt+F2显示Encode时，高度变长的问题 End
}

void EncodePanel::updateEncode(QString encode)
{
    m_encodeView->checkEncode(encode);
}

