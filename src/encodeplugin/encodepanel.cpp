/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:      zhangmeng <zhangmeng@uniontech.com>
 *
 * Maintainer:  zhangmeng <zhangmeng@uniontech.com>
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
#include "service.h"

#include <QScroller>
#include <QVBoxLayout>
#include <QDesktopWidget>

#include <DLog>
#include <DTitlebar>

EncodePanel::EncodePanel(QWidget *parent)
    : RightPanel(parent), m_encodeView(new EncodeListView(this))
{
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

/*******************************************************************************
 1. @函数:    updateEncode
 2. @作者:    ut001121 zhangmeng
 3. @日期:    2020-08-11
 4. @说明:    更新编码
*******************************************************************************/
void EncodePanel::updateEncode(QString encode)
{
    m_encodeView->checkEncode(encode);
}

/*******************************************************************************
 1. @函数:    resizeEvent
 2. @作者:    ut000438 王亮
 3. @日期:    2021-01-14
 4. @说明:    根据虚拟键盘高度，动态调整编码列表插件面板高度
*******************************************************************************/
void EncodePanel::resizeEvent(QResizeEvent *event)
{
    bool isTabletMode = IS_TABLET_MODE;
    // 非平板模式下不处理
    if (!isTabletMode) {
        return RightPanel::resizeEvent(event);
    }

    int statusbarHeight = 0;

    //先通过 QDBusInterface QDBus::AutoDetect 设置状态栏接口
    QDBusInterface interface(DUE_STATUSBAR_DBUS_NAME, DUE_STATUSBAR_DBUS_PATH, DUE_STATUSBAR_DBUS_NAME);
    if (interface.isValid()) {
        //判断接口是否有效，有效，调用接口获取状态栏高度
        QDBusMessage msg = interface.call(QDBus::AutoDetect, "height");

        if (QDBusMessage::ReplyMessage == msg.type()) {
            qInfo() << "get statusbar height Success!";
            QList<QVariant> list = msg.arguments();
            statusbarHeight = list.takeFirst().toInt();
            qInfo() << "dockHeight: " << statusbarHeight << endl;
        } else {
            qInfo() << "get statusbar height Fail!" << msg.errorMessage();
        }
    }

    Service *service = Service::instance();

    int titleBarHeight = Service::instance()->getTitleBarHeight();
    if (service->isVirtualKeyboardShow()) {
        int keyboardHeight = service->getVirtualKeyboardHeight();
        m_encodeView->setFixedHeight(QApplication::desktop()->geometry().height() - keyboardHeight - titleBarHeight - statusbarHeight);
    } else {
        // 获取标题栏高度
        m_encodeView->setFixedHeight(QApplication::desktop()->geometry().height() - titleBarHeight - statusbarHeight);
    }
}
