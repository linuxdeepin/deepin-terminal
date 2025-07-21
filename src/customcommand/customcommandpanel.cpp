// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "customcommandpanel.h"
#include "customcommandoptdlg.h"
#include "shortcutmanager.h"
#include "service.h"

#include <DGroupBox>
#include <DVerticalLine>
#include <DLog>
#include <DDialog>
#include <DHiDPIHelper>

#include <QPixmap>
#include <QEvent>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>

Q_DECLARE_LOGGING_CATEGORY(customcommand)

CustomCommandPanel::CustomCommandPanel(QWidget *parent) : CommonPanel(parent)
{
    qCDebug(customcommand) << "Creating CustomCommandPanel";
    Utils::set_Object_Name(this);
    initUI();
}
CustomCommandPanel::~CustomCommandPanel()
{
    qCDebug(customcommand) << "Destroying CustomCommandPanel";
    if (m_pdlg) {
        qCDebug(customcommand) << "m_pdlg is not null, deleting";
        delete m_pdlg;
        m_pdlg = nullptr;
    }
}

void CustomCommandPanel::showCurSearchResult()
{
    qCDebug(customcommand) << "Showing search results";
    QString strTxt = m_searchEdit->text();
    if (strTxt.isEmpty()) {
        qCDebug(customcommand) << "Search text is empty";
        return;
    }

    qCInfo(customcommand) << "Searching for:" << strTxt;
    emit showSearchResult(strTxt);
}

void CustomCommandPanel::showAddCustomCommandDlg()
{
    // qCDebug(customcommand) << "Enter showAddCustomCommandDlg";
    if (m_pushButton->hasFocus()) {
        qCInfo(customcommand) << "The Add command button has focus to click on!";
        m_bpushButtonHaveFocus = true;
    } else {
        qCInfo(customcommand) << "The Add command button has no focus to prohibit clicking!";
        m_bpushButtonHaveFocus = false;
    }

    if (m_pdlg) {
        qCDebug(customcommand) << "m_pdlg exists, deleting";
        delete m_pdlg;
        m_pdlg = nullptr;
    }

    // 弹窗显示
    Service::instance()->setIsDialogShow(window(), true);

    m_pdlg = new CustomCommandOptDlg(CustomCommandOptDlg::CCT_ADD, nullptr, this);
    m_pdlg->setObjectName("CustomAddCommandDialog");//Add by ut001000 renfeixiang 2020-08-14
    connect(m_pdlg, &CustomCommandOptDlg::finished, this, &CustomCommandPanel::onAddCommandResponse);
    m_pdlg->show();
}

void CustomCommandPanel::doCustomCommand(const QString &key)
{
    qCDebug(customcommand) << "Executing custom command with key:" << key;
    QAction *item = ShortcutManager::instance()->findActionByKey(key);
    if(!item) {
        qCWarning(customcommand) << "No command found for key:" << key;
        return;
    }

    QString strCommand = item->data().toString();
    if (!strCommand.endsWith('\n')) {
        qCDebug(customcommand) << "Appending newline to command";
        strCommand.append('\n');
    }

    qCInfo(customcommand) << "Executing command:" << strCommand;
    emit handleCustomCurCommand(strCommand);
}

void CustomCommandPanel::onFocusOut(Qt::FocusReason type)
{
    // qCDebug(customcommand) << "Enter onFocusOut";
    if ((Qt::TabFocusReason == type) || (Qt::NoFocusReason == type)) {
        qCDebug(customcommand) << "TabFocusReason or NoFocusReason";
        // 下一个 或 列表为空， 焦点定位到添加按钮上
        m_pushButton->setFocus();
        m_cmdListWidget->clearIndex();
        qCInfo(customcommand) << "Set the focus to the Add command button";
    } else if (Qt::BacktabFocusReason == type) {
        qCDebug(customcommand) << "BacktabFocusReason";
        // 判断是否可见，可见设置焦点
        if (m_searchEdit->isVisible()) {
            qCDebug(customcommand) << "search edit is visible";
            m_searchEdit->lineEdit()->setFocus();
            m_cmdListWidget->clearIndex();
            qCInfo(customcommand) << "Set the focus to the Search edit";
        }
    }
}

void CustomCommandPanel::onAddCommandResponse(int result)
{
        // qCDebug(customcommand) << "Enter onAddCommandResponse";
        // 弹窗隐藏或消失
        Service::instance()->setIsDialogShow(window(), false);
        if (QDialog::Accepted == result) {
            qCDebug(customcommand) << "dialog accepted";
            QAction *newAction = m_pdlg->getCurCustomCmd();
            // 新增快捷键 => 显示在列表中使用大写 down2up dzw 20201215
            m_cmdListWidget->addItem(ItemFuncType_Item, newAction->text(), Utils::converDownToUp(newAction->shortcut().toString()));
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
            qCDebug(customcommand) << "push button had focus, restoring";
            m_pushButton->setFocus(Qt::TabFocusReason);
        }
}

void CustomCommandPanel::refreshCmdPanel()
{
    qCDebug(customcommand) << "Refreshing command panel";
    clearSearchInfo();
    ShortcutManager::instance()->fillCommandListData(m_cmdListWidget);
    qCDebug(customcommand) << "Command list updated, count:" << m_cmdListWidget->count();
    refreshCmdSearchState();
}

void CustomCommandPanel::refreshCmdSearchState()
{
    // qCDebug(customcommand) << "Enter refreshCmdSearchState";
    if (m_cmdListWidget->count() >= 2) {
        qCDebug(customcommand) << "command list count >= 2";
        /************************ Add by m000743 sunchengxi 2020-04-22:自定义命令搜索显示异常 Begin************************/
        m_searchEdit->clearEdit();
        /************************ Add by m000743 sunchengxi 2020-04-22:自定义命令搜索显示异常  End ************************/
        m_searchEdit->show();
    } else {
        qCDebug(customcommand) << "command list count < 2";
        m_searchEdit->hide();
    }

    static bool hasStretch = false;
    if (m_cmdListWidget->count() <= 0) {
        qCDebug(customcommand) << "command list count <= 0";
        m_textLabel->show();
        m_imageLabel->show();
        if (hasStretch == false) {
            qCDebug(customcommand) << "hasStretch is false, adding stretch";
            dynamic_cast<QVBoxLayout*>(this->layout())->insertLayout(3, m_backLayout);
            dynamic_cast<QVBoxLayout*>(this->layout())->insertStretch(4);
            hasStretch = true;
        }
    } else {
        qCDebug(customcommand) << "command list count > 0";
        m_textLabel->hide();
        m_imageLabel->hide();
        if (hasStretch == true) {
            qCDebug(customcommand) << "hasStretch is true, removing stretch";
            this->layout()->removeItem(this->layout()->itemAt(3));
            this->layout()->removeItem(this->layout()->itemAt(3));
            hasStretch = false;
        }
    }
}

void CustomCommandPanel::setFocusInPanel()
{
    // qCDebug(customcommand) << "Enter setFocusInPanel";
    if (m_searchEdit->isVisible()) {
        qCDebug(customcommand) << "search edit is visible";
        // 搜索框在
        m_searchEdit->lineEdit()->setFocus();
    } else if (m_cmdListWidget->isVisible() && m_cmdListWidget->count() != 0) {
        qCDebug(customcommand) << "command list widget is visible and has items";
        // 列表数据不为0
        m_cmdListWidget->setFocus();
    } else if (m_pushButton->isVisible()) {
        qCDebug(customcommand) << "push button is visible";
        // 添加按钮下
        m_pushButton->setFocus();
    } else {
        qCWarning(customcommand) << "focus error unkown reason";
    }
}

void CustomCommandPanel::initUI()
{
    qCDebug(customcommand) << "Initializing CustomCommandPanel UI";
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);

    m_searchEdit = new DSearchEdit(this);
    //fix bug#64976 按tab键循环切换到右上角X按钮时继续按tab键，焦点不能切换到搜索框
    m_searchEdit->setFocusPolicy(Qt::StrongFocus);
    m_searchEdit->setFocusProxy(m_searchEdit->lineEdit());
    m_searchEdit->setObjectName("CustomSearchEdit");//Add by ut001000 renfeixiang 2020-08-13
    m_searchEdit->setClearButtonEnabled(true);
    DFontSizeManager::instance()->bind(m_searchEdit, DFontSizeManager::T6);

    m_cmdListWidget = new ListView(ListType_Custom, this);
    m_cmdListWidget->setObjectName("CustomCommandListWidget");//Add by ut001000 renfeixiang 2020-08-13

    m_pushButton = new DPushButton(this);
    m_pushButton->setObjectName("CustomAddCommandButton");//Add by ut001000 renfeixiang 2020-08-13
    m_pushButton->setText(tr("Add Command"));

    m_textLabel = new DLabel(this);
    m_textLabel->resize(136, 18);
    m_textLabel->setText(tr("No commands yet"));
    DFontSizeManager::instance()->bind(m_textLabel, DFontSizeManager::T6);

    m_imageLabel = new DLabel(this);
    m_imageLabel->setFixedSize(QSize(88, 88));
    m_imageLabel->setPixmap(DHiDPIHelper::loadNxPixmap(":/other/command.svg"));

    QHBoxLayout *textLayout = new QHBoxLayout();
    textLayout->addStretch();
    textLayout->addWidget(m_textLabel);
    textLayout->addStretch();

    QHBoxLayout *imageLayout = new QHBoxLayout();
    imageLayout->addStretch();
    imageLayout->addWidget(m_imageLabel);
    imageLayout->addStretch();

    m_backLayout = new QVBoxLayout();
    m_backLayout->setContentsMargins(0, 0, 0, 0);
    m_backLayout->addStretch();
    m_backLayout->addLayout(imageLayout);
    m_backLayout->setSpacing(SPACEWIDTH);
    m_backLayout->addLayout(textLayout);
    m_backLayout->addStretch();
    m_backLayout->setSpacing(0);
    m_backLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addSpacing(10);
    btnLayout->addWidget(m_pushButton);
    btnLayout->addSpacing(10);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setSpacing(0);
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
