// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "remotemanagementpanel.h"
#include "service.h"
#include "utils.h"

#include <DLog>
#include <DHiDPIHelper>

RemoteManagementPanel::RemoteManagementPanel(QWidget *parent) : CommonPanel(parent)
{
    Utils::set_Object_Name(this);
    initUI();
}

void RemoteManagementPanel::refreshPanel()
{
    // 清空搜索框
    clearSearchInfo();
    // 初始化列表
    ServerConfigManager::instance()->refreshServerList(ServerConfigManager::PanelType_Manage, m_listWidget);
    // 刷新搜索框状态
    refreshSearchState();
}

void RemoteManagementPanel::setFocusInPanel()
{
    qInfo() << "RemoteManagementPanel focus in Panel.";
    if (m_searchEdit->isVisible()) {
        // 搜索框显示
        // 设置焦点
        m_searchEdit->lineEdit()->setFocus();
    } else if (m_listWidget->isVisible()) {
        // 列表显示
        /******** Modify by ut000610 daizhengwen 2020-07-27:bug#39775 Begin***************/
        // 将焦点设置在列表里的第一项
        m_listWidget->setCurrentIndex(0);
        /********************* Modify by ut000610 daizhengwen End ************************/
    } else {
        // 将焦点显示在添加按钮上
        m_pushButton->setFocus();
    }
}

void RemoteManagementPanel::setFocusBack(const QString &strGroup)
{
    qInfo() << "RemoteManagementPanel return from RemoteManageGroup";
    // 返回前判断之前是否要有焦点
    if (m_listWidget->getFocusState()) {
        // 要有焦点
        // 找到分组的新位置
        int index = m_listWidget->indexFromString(strGroup, ItemFuncType_Group);
        if (index < 0) {
            // 小于0代表没找到 获取下一个
            index = m_listWidget->getNextIndex(m_listWidget->currentIndex());
        }

        if (index >= 0) {
            // 找得到, 设置焦点
            m_listWidget->setCurrentIndex(index);
        } else {
            // 没找到焦点设置到添加按钮
            m_pushButton->setFocus();
        }
    }
    // 不要焦点
    else {
        MainWindow *w = Utils::getMainWindow(this);
        if(w)
            w->focusCurrentPage();
    }
}

void RemoteManagementPanel::clearListFocus()
{
    m_pushButton->clearFocus();
    m_listWidget->clearFocus();
    m_searchEdit->clearFocus();
    m_listWidget->clearIndex();
}

int RemoteManagementPanel::getListIndex()
{
    return m_listWidget->currentIndex();
}

void RemoteManagementPanel::refreshSearchState()
{
    if (m_listWidget->count() >= 2) {
        /************************ Add by m000743 sunchengxi 2020-04-22:搜索显示异常 Begin************************/
        m_searchEdit->clearEdit();
        /************************ Add by m000743 sunchengxi 2020-04-22:搜索显示异常  End ************************/
        m_searchEdit->show();
    } else {
        m_searchEdit->hide();
    }

    static bool hasStretch = false;
    if (m_listWidget->count() <= 0) {
        m_textLabel->show();
        m_imageLabel->show();
        if (hasStretch == false) {
            dynamic_cast<QVBoxLayout*>(this->layout())->insertLayout(3, m_backLayout);
            dynamic_cast<QVBoxLayout*>(this->layout())->insertStretch(4);
            hasStretch = true;
        }
    } else {
        m_textLabel->hide();
        m_imageLabel->hide();
        if (hasStretch == true) {
            this->layout()->removeItem(this->layout()->itemAt(3));
            this->layout()->removeItem(this->layout()->itemAt(3));
            hasStretch = false;
        }
    }
}

void RemoteManagementPanel::onItemClicked(const QString &key)
{
    // 获取远程信息
    ServerConfig *remote = ServerConfigManager::instance()->getServerConfig(key);
    if (nullptr != remote)
        emit doConnectServer(remote);
    else
        qInfo() << "can't connect to remote" << key;
}

void RemoteManagementPanel::showCurSearchResult()
{
    QString strTxt = m_searchEdit->text();
    if (strTxt.isEmpty())
        return;

    emit showSearchPanel(strTxt);
}

void RemoteManagementPanel::showAddServerConfigDlg()
{
    // 判断控件是否有焦点
    bool focusState = m_pushButton->hasFocus();
    // 弹窗显示
    Service::instance()->setIsDialogShow(window(), true);

    ServerConfigOptDlg *dlg = new ServerConfigOptDlg(ServerConfigOptDlg::SCT_ADD, nullptr, this);
    connect(dlg, &ServerConfigOptDlg::finished, this, [ = ](int result) {
        // 弹窗隐藏或消失
        Service::instance()->setIsDialogShow(window(), false);
        // 先判断是否要焦点
        if (focusState) {
            // 让焦点在平面上
            setFocus();
            // 添加完，将焦点设置在添加按钮上
            m_pushButton->setFocus();
        }
        if (QDialog::Accepted == result) {
            int index = m_listWidget->indexFromString(dlg->getServerName());
            m_listWidget->setScroll(index);
        }
    });
    dlg->show();
}

void RemoteManagementPanel::showAddGroupConfigDlg(const QString &groupName)
{
    // 判断控件是否有焦点55
    bool focusState = m_pushButton->hasFocus();
    // 弹窗显示
    Service::instance()->setIsDialogShow(window(), true);

    GroupConfigOptDlg *dlg = new GroupConfigOptDlg(groupName);
    connect(dlg, &ServerConfigOptDlg::finished, this, [ = ](int result) {
        // 弹窗隐藏或消失
        Service::instance()->setIsDialogShow(window(), false);
        // 先判断是否要焦点
        if (focusState) {
            // 让焦点在平面上
            setFocus();
            // 添加完，将焦点设置在添加按钮上
            m_addGroupButton->setFocus();
        }
        if (QDialog::Accepted == result) {
            refreshPanel();
        }
    });
    dlg->show();
}

void RemoteManagementPanel::initUI()
{
    this->setBackgroundRole(QPalette::Base);
    this->setAutoFillBackground(true);

    m_searchEdit = new DSearchEdit(this);
    //fix bug#64976 按tab键循环切换到右上角X按钮时继续按tab键，焦点不能切换到搜索框
    m_searchEdit->setFocusPolicy(Qt::StrongFocus);
    m_searchEdit->setFocusProxy(m_searchEdit->lineEdit());
    m_searchEdit->setObjectName("RemoteSearchEdit");
    m_listWidget = new ListView(ListType_Remote, this);
    m_listWidget->setObjectName("RemoteManageListWidget");
    m_addGroupButton = new DPushButton(this);
    m_addGroupButton->setObjectName("AddServerGroupButton");
    m_pushButton = new DPushButton(this);
    m_pushButton->setObjectName("RemoteAddPushButton");

    m_searchEdit->setFixedHeight(COMMONHEIGHT);
    m_searchEdit->setClearButtonEnabled(true);

    m_addGroupButton->setFixedHeight(COMMONHEIGHT);
    m_addGroupButton->setText(tr("Add Group"));
    m_pushButton->setFixedHeight(COMMONHEIGHT);
    m_pushButton->setText(tr("Add Server"));

    m_textLabel = new DLabel(this);
    m_textLabel->resize(136, 18);
    m_textLabel->setText(tr("No servers yet"));
    DFontSizeManager::instance()->bind(m_textLabel, DFontSizeManager::T6);

    m_imageLabel = new DLabel(this);
    m_imageLabel->setFixedSize(QSize(88, 88));
    m_imageLabel->setPixmap(DHiDPIHelper::loadNxPixmap(":/other/server_group.svg"));

    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->setContentsMargins(0, 0, 0, 0);
    hlayout->addSpacing(SPACEWIDTH);
    hlayout->addWidget(m_searchEdit);
    hlayout->addSpacing(SPACEWIDTH);
    hlayout->setSpacing(0);
    hlayout->setMargin(0);

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
    m_backLayout->setMargin(0);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setContentsMargins(0, 0, 0, 0);
    btnLayout->addSpacing(SPACEWIDTH);
    btnLayout->addWidget(m_pushButton);
    btnLayout->addSpacing(SPACEWIDTH);
    btnLayout->addWidget(m_addGroupButton);
    btnLayout->addSpacing(SPACEWIDTH);
    btnLayout->setSpacing(0);
    btnLayout->setMargin(0);

    QVBoxLayout *vlayout = new QVBoxLayout();
    vlayout->setContentsMargins(0, 0, 0, 0);
    vlayout->addSpacing(SPACEHEIGHT);
    vlayout->addLayout(hlayout);
    vlayout->addWidget(m_listWidget);
    vlayout->addLayout(btnLayout);
    vlayout->addSpacing(SPACEHEIGHT);
    vlayout->setMargin(0);
    vlayout->setSpacing(SPACEHEIGHT);
    setLayout(vlayout);

    connect(m_searchEdit, &DSearchEdit::returnPressed, this, &RemoteManagementPanel::showCurSearchResult);
    connect(m_pushButton, &DPushButton::clicked, this, &RemoteManagementPanel::showAddServerConfigDlg);
    connect(m_addGroupButton, &DPushButton::clicked, this, [this] { showAddGroupConfigDlg(); });
    connect(m_listWidget, &ListView::itemClicked, this, &RemoteManagementPanel::onItemClicked);
    connect(m_listWidget, &ListView::groupClicked, this, &RemoteManagementPanel::showGroupPanel);
    connect(m_listWidget, &ListView::groupModify, this, &RemoteManagementPanel::showAddGroupConfigDlg);
    connect(m_listWidget, &ListView::listItemCountChange, this, &RemoteManagementPanel::refreshSearchState);
    connect(ServerConfigManager::instance(), &ServerConfigManager::refreshList, this, [ = ]() {
        if (m_isShow)
            refreshPanel();
    });
    connect(m_listWidget, &ListView::focusOut, this, [ = ](Qt::FocusReason type) {
        if (Qt::TabFocusReason == type || Qt::NoFocusReason == type) {
            // 下一个 或 列表为空， 焦点定位到添加按钮上
            m_pushButton->setFocus();
            m_listWidget->clearIndex();
            qInfo() << "set focus on add pushButton";
        } else if (Qt::BacktabFocusReason == type) {
            // 判断是否可见，可见设置焦点
            if (m_searchEdit->isVisible()) {
                m_searchEdit->lineEdit()->setFocus();
                m_listWidget->clearIndex();
                qInfo() << "set focus on add search edit";
            }
        }

    });
}
