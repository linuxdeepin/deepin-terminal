/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:     sunchengxi <sunchengxi@uniontech.com>
 *
 * Maintainer: sunchengxi <sunchengxi@uniontech.com>
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
#include "customcommandpanel.h"
#include "customcommandoptdlg.h"
#include "shortcutmanager.h"
#include "service.h"

#include <DGroupBox>
#include <DVerticalLine>
#include <DLog>
#include <DDialog>
#include <DTitlebar>

#include <QPixmap>
#include <QEvent>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QDesktopWidget>

CustomCommandPanel::CustomCommandPanel(QWidget *parent) : CommonPanel(parent)
{
    Utils::set_Object_Name(this);
    initUI();
}
CustomCommandPanel::~CustomCommandPanel()
{
    if (m_pdlg) {
        delete m_pdlg;
        m_pdlg = nullptr;
    }
}

/*******************************************************************************
 1. @函数:    showCurSearchResult
 2. @作者:    sunchengxi
 3. @日期:    2020-08-05
 4. @说明:    处理显示搜索结果的槽函数
*******************************************************************************/
void CustomCommandPanel::showCurSearchResult()
{
    QString strTxt = m_searchEdit->text();
    if (strTxt.isEmpty()) {
        return;
    }
    emit showSearchResult(strTxt);
}

/*******************************************************************************
 1. @函数:    showAddCustomCommandDlg
 2. @作者:    sunchengxi
 3. @日期:    2020-08-05
 4. @说明:    增加自定义命令操作的槽函数
*******************************************************************************/
void CustomCommandPanel::showAddCustomCommandDlg()
{
    qDebug() <<  __FUNCTION__ << __LINE__;

    if (m_pushButton->hasFocus()) {
        qDebug() << "------------------------hasFocus";
        m_bpushButtonHaveFocus = true;
    } else {
        qDebug() << "------------------------ not         hasFocus";
        m_bpushButtonHaveFocus = false;
    }

    if (m_pdlg) {
        delete m_pdlg;
        m_pdlg = nullptr;
    }

    // 弹窗显示
    Service::instance()->setIsDialogShow(window(), true);

    m_pdlg = new CustomCommandOptDlg(CustomCommandOptDlg::CCT_ADD, nullptr, this);
    m_pdlg->setObjectName("CustomAddCommandDialog");//Add by ut001000 renfeixiang 2020-08-14
    connect(m_pdlg, &CustomCommandOptDlg::finished, this, [ &](int result) {
        // 弹窗隐藏或消失
        Service::instance()->setIsDialogShow(window(), false);

        qDebug() << "finished" << result;

        if (QDialog::Accepted == result) {
            qDebug() << "Accepted";
            QAction *newAction = m_pdlg->getCurCustomCmd();
            m_cmdListWidget->addItem(ItemFuncType_Item, newAction->text(), newAction->shortcut().toString());
            /************************ Add by m000743 sunchengxi 2020-04-20:解决自定义命令无法添加 Begin************************/
            ShortcutManager::instance()->addCustomCommand(*newAction);
            /************************ Add by m000743 sunchengxi 2020-04-20:解决自定义命令无法添加 End  ************************/

            emit Service::instance()->refreshCommandPanel("", "");

            refreshCmdSearchState();
            /******** Modify by m000714 daizhengwen 2020-04-10: 滚动条滑至最底端****************/
            int index = m_cmdListWidget->indexFromString(newAction->text());
            m_cmdListWidget->setScroll(index);
            /********************* Modify by m000714 daizhengwen End ************************/

        }

        if (m_bpushButtonHaveFocus) {
            m_pushButton->setFocus(Qt::TabFocusReason);
        }

    });
    m_pdlg->show();
}

/*******************************************************************************
 1. @函数:    doCustomCommand
 2. @作者:    sunchengxi
 3. @日期:    2020-08-05
 4. @说明:    执行自定义命令
*******************************************************************************/
void CustomCommandPanel::doCustomCommand(const QString &key)
{
    qDebug() <<  __FUNCTION__ << __LINE__;
    QAction *item = ShortcutManager::instance()->findActionByKey(key);

    QString strCommand = item->data().toString();
    if (!strCommand.endsWith('\n')) {
        strCommand.append('\n');
    }
    emit handleCustomCurCommand(strCommand);
}

/*******************************************************************************
 1. @函数:    onFocusOut
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-08-04
 4. @说明:    处理焦点出列表的事件
*******************************************************************************/
void CustomCommandPanel::onFocusOut(Qt::FocusReason type)
{
    if ((Qt::TabFocusReason == type) || (Qt::NoFocusReason == type)) {
        // 下一个 或 列表为空， 焦点定位到添加按钮上
        m_pushButton->setFocus();
        m_cmdListWidget->clearIndex();
        qDebug() << "set focus on add pushButton";
    } else if (Qt::BacktabFocusReason == type) {
        // 判断是否可见，可见设置焦点
        if (m_searchEdit->isVisible()) {
            m_searchEdit->lineEdit()->setFocus();
            m_cmdListWidget->clearIndex();
            qDebug() << "set focus on add search edit";
        }
    }
}

/*******************************************************************************
 1. @函数:    refreshCmdPanel
 2. @作者:    sunchengxi
 3. @日期:    2020-08-05
 4. @说明:    自定义命令面刷新
*******************************************************************************/
void CustomCommandPanel::refreshCmdPanel()
{
    qDebug() <<  __FUNCTION__ << __LINE__;
    clearSearchInfo();
    ShortcutManager::instance()->fillCommandListData(m_cmdListWidget);
    refreshCmdSearchState();
}

/*******************************************************************************
 1. @函数:    refreshCmdSearchState
 2. @作者:    sunchengxi
 3. @日期:    2020-08-05
 4. @说明:    自定义命令面板搜索框显示布局控制
*******************************************************************************/
void CustomCommandPanel::refreshCmdSearchState()
{
    qDebug() << __FUNCTION__ << m_cmdListWidget->count() << endl;
    if (m_cmdListWidget->count() >= 2) {
        /************************ Add by m000743 sunchengxi 2020-04-22:自定义命令搜索显示异常 Begin************************/
        m_searchEdit->clearEdit();
        /************************ Add by m000743 sunchengxi 2020-04-22:自定义命令搜索显示异常  End ************************/
        m_searchEdit->show();
    } else {
        m_searchEdit->hide();
    }
}

/*******************************************************************************
 1. @函数:    setFocusInPanel
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-08-03
 4. @说明:    将焦点设置到平面
 若有搜索框，焦点进搜索框
 若没搜索框，焦点进列表
 若没列表，焦点进添加按钮
*******************************************************************************/
void CustomCommandPanel::setFocusInPanel()
{
    qDebug() <<  __FUNCTION__ << __LINE__;
    if (m_searchEdit->isVisible()) {
        // 搜索框在
        m_searchEdit->lineEdit()->setFocus();
    } else if (m_cmdListWidget->isVisible() && m_cmdListWidget->count() != 0) {
        // 列表数据不为0
        m_cmdListWidget->setFocus();
    } else if (m_pushButton->isVisible()) {
        // 添加按钮下
        m_pushButton->setFocus();
    } else {
        qDebug() << "focus error unkown reason";
    }
}

/*******************************************************************************
 1. @函数:    initUI
 2. @作者:    sunchengxi
 3. @日期:    2020-08-05
 4. @说明:    自定义命令面板界面初始化
*******************************************************************************/
void CustomCommandPanel::initUI()
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);

    m_searchEdit = new DSearchEdit(this);
    m_searchEdit->setObjectName("CustomSearchEdit");//Add by ut001000 renfeixiang 2020-08-13
    m_searchEdit->setClearButtonEnabled(true);
    DFontSizeManager::instance()->bind(m_searchEdit, DFontSizeManager::T6);

    m_cmdListWidget = new ListView(ListType_Custom, this);
    m_cmdListWidget->setObjectName("CustomCommandListWidget");//Add by ut001000 renfeixiang 2020-08-13

    m_pushButton = new DPushButton(this);
    m_pushButton->setObjectName("CustomAddCommandButton");//Add by ut001000 renfeixiang 2020-08-13
    m_pushButton->setFixedHeight(36);
    m_pushButton->setText(tr("Add Command"));

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addSpacing(10);
    btnLayout->addWidget(m_pushButton);
    btnLayout->addSpacing(10);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setSpacing(0);
    hLayout->setMargin(0);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->addSpacing(10);
    hLayout->addWidget(m_searchEdit);
    hLayout->addSpacing(10);

    QVBoxLayout *vLayout = new QVBoxLayout(this);
    m_vlayout = vLayout;
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(10);
    vLayout->addSpacing(10);
    vLayout->addLayout(hLayout);
    vLayout->addWidget(m_cmdListWidget);
    vLayout->addLayout(btnLayout);
    vLayout->addSpacing(10);
    setLayout(vLayout);

    connect(m_searchEdit, &DSearchEdit::returnPressed, this, &CustomCommandPanel::showCurSearchResult);
    connect(m_pushButton, &DPushButton::clicked, this, &CustomCommandPanel::showAddCustomCommandDlg);
    connect(m_cmdListWidget, &ListView::itemClicked, this, &CustomCommandPanel::doCustomCommand);
    connect(m_cmdListWidget, &ListView::listItemCountChange, this, &CustomCommandPanel::refreshCmdSearchState);
    connect(m_cmdListWidget, &ListView::focusOut, this, &CustomCommandPanel::onFocusOut);
}

/*******************************************************************************
 1. @函数:    resizeEvent
 2. @作者:    ut000438 王亮
 3. @日期:    2021-01-18
 4. @说明:    根据虚拟键盘高度，动态调整编码列表插件面板高度
*******************************************************************************/
void CustomCommandPanel::resizeEvent(QResizeEvent *event)
{
    bool isTabletMode = IS_TABLET_MODE;
    // 非平板模式下不处理
    if (!isTabletMode) {
        return CommonPanel::resizeEvent(event);
    }

    //防止调用this->resize后循环触发resizeEvent
    if (m_isResizeBySelf) {
        m_isResizeBySelf = false;
        return;
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

    // 获取标题栏高度
    int titleBarHeight = service->getTitleBarHeight();
    int topPanelHeight = 0;
    if (service->isVirtualKeyboardShow()) {
        int keyboardHeight = service->getVirtualKeyboardHeight();
        topPanelHeight = QApplication::desktop()->geometry().height() - keyboardHeight - titleBarHeight - statusbarHeight;
    } else {
        topPanelHeight = QApplication::desktop()->availableGeometry().height() - titleBarHeight - statusbarHeight;
    }

    int topPanelWidth = this->width();
    this->resize(topPanelWidth, topPanelHeight);

    m_isResizeBySelf = true;
}
