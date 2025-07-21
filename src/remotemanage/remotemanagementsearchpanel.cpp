// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "remotemanagementsearchpanel.h"
#include "utils.h"
#include "listview.h"
#include "iconbutton.h"
#include "mainwindow.h"

#include <DGuiApplicationHelper>

#include <QKeyEvent>
#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(remotemanage)

RemoteManagementSearchPanel::RemoteManagementSearchPanel(QWidget *parent) : CommonPanel(parent)
{
    qCDebug(remotemanage) << "RemoteManagementSearchPanel constructor";
    Utils::set_Object_Name(this);
    initUI();
}

void RemoteManagementSearchPanel::initUI()
{
    qCDebug(remotemanage) << "Enter initUI";
    this->setBackgroundRole(QPalette::Base);
    this->setAutoFillBackground(true);

    m_rebackButton = new IconButton(this);
    m_rebackButton->setObjectName("RemoteSearchRebackButton");
    m_rebackButton->setIcon(DStyle::StandardPixmap::SP_ArrowLeave);
    m_rebackButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    m_rebackButton->setFocusPolicy(Qt::TabFocus);

    m_listWidget = new ListView(ListType_Remote, this);
    m_listWidget->setObjectName("RemoteSearchListWidget");
    m_label = new DLabel(this);
    m_label->setObjectName("RemoteSearchFilterLabel");//Add by ut001000 renfeixiang 2020-08-13;
    m_label->setAlignment(Qt::AlignCenter);
    // 字体颜色随主题变化而变化
    DPalette palette = m_label->palette();
    QColor color;
    if (DGuiApplicationHelper::DarkType == DGuiApplicationHelper::instance()->themeType()) {
        qCDebug(remotemanage) << "using dark theme color";
        color = QColor::fromRgb(192, 198, 212, 102);
    } else {
        qCDebug(remotemanage) << "using light theme color";
        color = QColor::fromRgb(85, 85, 85, 102);
    }

    palette.setBrush(QPalette::Text, color);
    m_label->setPalette(palette);

    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->addSpacing(SPACEWIDTH);
    hlayout->addWidget(m_rebackButton);
    hlayout->addWidget(m_label, 0, Qt::AlignCenter);
    hlayout->setSpacing(0);
    hlayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *vlayout = new QVBoxLayout();
    vlayout->addSpacing(SPACEHEIGHT);
    vlayout->addLayout(hlayout);
    vlayout->addWidget(m_listWidget);
    vlayout->setContentsMargins(0, 0, 0, 0);
    vlayout->setSpacing(SPACEHEIGHT);
    setLayout(vlayout);

    // 返回键被点击 搜索界面，返回焦点返回搜索框
    connect(m_rebackButton, &DIconButton::clicked, this, &RemoteManagementSearchPanel::rebackPrevPanel);
    connect(m_rebackButton, &IconButton::preFocus, this, &RemoteManagementSearchPanel::rebackPrevPanel);
    connect(m_rebackButton, &IconButton::focusOut, this, &RemoteManagementSearchPanel::onFocusOutList);
    connect(m_listWidget, &ListView::itemClicked, this, &RemoteManagementSearchPanel::onItemClicked);
    connect(m_listWidget, &ListView::groupClicked, this, &RemoteManagementSearchPanel::showGroupPanel);
    connect(ServerConfigManager::instance(), &ServerConfigManager::refreshList, this, &RemoteManagementSearchPanel::onRefreshList);
    connect(m_listWidget, &ListView::focusOut, this, &RemoteManagementSearchPanel::handleListViewFocusOut);
    // 字体颜色随主题变化变化
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &RemoteManagementSearchPanel::handleThemeTypeChanged);
    qCDebug(remotemanage) << "Exit initUI";
}

inline void RemoteManagementSearchPanel::handleListViewFocusOut(Qt::FocusReason type)
{
    qCDebug(remotemanage) << "Enter RemoteManagementSearchPanel::handleListViewFocusOut";
    if (Qt::TabFocusReason == type) {
        qCDebug(remotemanage) << "tab focus reason";
        // tab 进入 +
        QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_Tab, Qt::MetaModifier);
        QApplication::sendEvent(Utils::getMainWindow(this), &keyPress);
        qCInfo(remotemanage) << "search panel focus on '+'";
        m_listWidget->clearIndex();
    } else if (Qt::BacktabFocusReason == type || Qt::NoFocusReason == type) {
        qCDebug(remotemanage) << "backtab or no focus reason";
        // shift + tab 返回 返回键               // 列表为空，也返回到返回键上
        m_rebackButton->setFocus();
        m_listWidget->clearIndex();
        qCInfo(remotemanage) << "search panel type" << type;
    }
}

inline void RemoteManagementSearchPanel::handleThemeTypeChanged(DGuiApplicationHelper::ColorType themeType)
{
    qCDebug(remotemanage) << "Enter RemoteManagementSearchPanel::handleThemeTypeChanged";
    DPalette palette = m_label->palette();
    QColor color;
    if (DGuiApplicationHelper::DarkType == themeType) {
        qCDebug(remotemanage) << "changing to dark theme";
        color = QColor::fromRgb(192, 198, 212, 102);
    } else {
        qCDebug(remotemanage) << "changing to light theme";
        color = QColor::fromRgb(85, 85, 85, 102);
    }

    palette.setBrush(QPalette::Text, color);
    m_label->setPalette(palette);
}

void RemoteManagementSearchPanel::refreshDataByGroupAndFilter(const QString &strGroup, const QString &strFilter)
{
    qCDebug(remotemanage) << "Enter refreshDataByGroupAndFilter, group:" << strGroup << "filter:" << strFilter;
    setSearchFilter(strFilter);
    m_isGroupOrNot = true;
    m_strGroupName = strGroup;
    m_strFilter = strFilter;
    m_listWidget->clearData();
    ServerConfigManager::instance()->refreshServerList(ServerConfigManager::PanelType_Search, m_listWidget, strFilter, strGroup);
    qCDebug(remotemanage) << "Exit refreshDataByGroupAndFilter";
}

void RemoteManagementSearchPanel::refreshDataByFilter(const QString &strFilter)
{
    qCDebug(remotemanage) << "Enter refreshDataByFilter, filter:" << strFilter;
    setSearchFilter(strFilter);
    m_isGroupOrNot = false;
    m_strFilter = strFilter;
    m_listWidget->clearData();
    ServerConfigManager::instance()->refreshServerList(ServerConfigManager::PanelType_Search, m_listWidget, strFilter);
    qCDebug(remotemanage) << "Exit refreshDataByFilter";
}

void RemoteManagementSearchPanel::onItemClicked(const QString &key)
{
    qCDebug(remotemanage) << "onItemClicked, key:" << key;
    // 获取远程信息
    ServerConfig *remote = ServerConfigManager::instance()->getServerConfig(key);
    if (nullptr != remote) {
        qCDebug(remotemanage) << "remote config found, emitting doConnectServer";
        emit doConnectServer(remote);
    } else {
        qCInfo(remotemanage) << "can't connect to remote" << key;
    }
}

void RemoteManagementSearchPanel::onFocusOutList(Qt::FocusReason type)
{
    // // qCDebug(remotemanage) << "Enter RemoteManagementSearchPanel::onFocusOutList";
    // 焦点切出，没值的时候
    if (Qt::TabFocusReason == type && 0 == m_listWidget->count()) {
        qCDebug(remotemanage) << "tab focus reason and list is empty";
        // tab 进入 +
        QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_Tab, Qt::MetaModifier);
        QApplication::sendEvent(Utils::getMainWindow(this), &keyPress);
        qCInfo(remotemanage) << "search panel focus to '+'";
    }
    qCDebug(remotemanage) << "Exit onRefreshList";
}

void RemoteManagementSearchPanel::onRefreshList()
{
    qCDebug(remotemanage) << "Enter onRefreshList";
    // 判断是否显示
    if (m_isShow) {
        qCDebug(remotemanage) << "panel is shown";
        if (m_isGroupOrNot) {
            qCDebug(remotemanage) << "refreshing group search list";
            // 刷新分组下搜索列表
            refreshDataByGroupAndFilter(m_strGroupName, m_strFilter);
        } else {
            qCDebug(remotemanage) << "refreshing global search list";
            // 刷新搜索列表
            refreshDataByFilter(m_strFilter);
        }
    }
}

void RemoteManagementSearchPanel::clearAllFocus()
{
    qCDebug(remotemanage) << "Enter RemoteManagementSearchPanel::clearAllFocus";
    m_rebackButton->clearFocus();
    m_listWidget->clearFocus();
    m_label->clearFocus();
}

void RemoteManagementSearchPanel::setFocusBack(const QString &strGroup, bool isFocusOn, int prevIndex)
{
    qCInfo(remotemanage) << "RemoteManagementSearchPanel return from RemoteManagementGroup.";
    // 返回前判断之前是否要有焦点
    if (isFocusOn) {
        qCDebug(remotemanage) << "focus is required";
        // 要有焦点
        // 找到分组的新位置
        int index = m_listWidget->indexFromString(strGroup, ItemFuncType_Group);
        if (index < 0) {
            qCDebug(remotemanage) << "group not found, getting next index";
            // 小于0代表没找到 获取下一个
            index = m_listWidget->getNextIndex(prevIndex);
        }

        if (index >= 0) {
            qCDebug(remotemanage) << "setting focus to list index:" << index;
            // 找得到, 设置焦点
            m_listWidget->setCurrentIndex(index);
        } else {
            qCDebug(remotemanage) << "setting focus to reback button";
            // 没找到焦点设置到添加按钮
            m_rebackButton->setFocus();
        }
    }
    // 不要焦点
    else {
        qCDebug(remotemanage) << "focus is not required";
        MainWindow *w = Utils::getMainWindow(this);
        if(w) {
            qCDebug(remotemanage) << "setting focus to current page";
            w->focusCurrentPage();
        }
    }
}

int RemoteManagementSearchPanel::getListIndex()
{
    // qCDebug(remotemanage) << "Enter RemoteManagementSearchPanel::getListIndex";
    return m_listWidget->currentIndex();
}

void RemoteManagementSearchPanel::setSearchFilter(const QString &filter)
{
    qCDebug(remotemanage) << "setSearchFilter, filter:" << filter;
    m_strFilter = filter;
    QString showText = filter;
    showText = Utils::getElidedText(m_label->font(), showText, ITEMMAXWIDTH, Qt::ElideMiddle);
    m_label->setText(QString("%1：%2").arg(tr("Search"), showText));
}
