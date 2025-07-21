// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "customcommandsearchrstpanel.h"
#include "customcommandoptdlg.h"
#include "shortcutmanager.h"
#include "iconbutton.h"
#include "listview.h"
#include "utils.h"
#include "mainwindow.h"

#include <DGuiApplicationHelper>
#include <DMessageBox>

#include <QAction>
#include <QKeyEvent>
#include <QApplication>
#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <QLoggingCategory>
Q_DECLARE_LOGGING_CATEGORY(customcommand)

CustomCommandSearchRstPanel::CustomCommandSearchRstPanel(QWidget *parent)
    : CommonPanel(parent)
{
    qCDebug(customcommand) << "Creating CustomCommandSearchRstPanel";
    Utils::set_Object_Name(this);
    initUI();
    qCDebug(customcommand) << "CustomCommandSearchRstPanel created";
}

void CustomCommandSearchRstPanel::initUI()
{
    qCDebug(customcommand) << "Initializing CustomCommandSearchRstPanel UI";
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);

    m_rebackButton = new IconButton(this);
    m_rebackButton->setObjectName("CustomRebackButton");
    m_backButton = m_rebackButton;
    m_backButton->setIcon(DStyle::StandardPixmap::SP_ArrowLeave);
    m_backButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    m_backButton->setFocusPolicy(Qt::TabFocus);

    m_label = new DLabel(this);
    m_label->setObjectName("CustomSearchFilterlabel");//Add by ut001000 renfeixiang 2020-08-13
    m_label->setAlignment(Qt::AlignCenter);
    // 字体颜色随主题变化变化
    DPalette palette = m_label->palette();
    QColor color;
    if (DGuiApplicationHelper::DarkType == DGuiApplicationHelper::instance()->themeType()) {
        qCDebug(customcommand) << "Dark theme type";
        color = QColor::fromRgb(192, 198, 212, 102);
    } else {
        qCDebug(customcommand) << "Light theme type";
        color = QColor::fromRgb(85, 85, 85, 102);
    }

    palette.setBrush(QPalette::Text, color);
    m_label->setPalette(palette);

    m_cmdListWidget = new ListView(ListType_Custom, this);
    m_cmdListWidget->setObjectName("CustomcmdSearchListWidget");//Add by ut001000 renfeixiang 2020-08-14

    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->addSpacing(10);
    hlayout->addWidget(m_backButton);
    // 搜索框居中显示
    hlayout->addWidget(m_label, 0, Qt::AlignCenter);
    hlayout->setSpacing(0);
    hlayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *vlayout = new QVBoxLayout();
    vlayout->addSpacing(10);
    vlayout->addLayout(hlayout);
    vlayout->addWidget(m_cmdListWidget);
    vlayout->setContentsMargins(0, 0, 0, 0);
    vlayout->setSpacing(10);
    setLayout(vlayout);

    connect(m_cmdListWidget, &ListView::itemClicked, this, &CustomCommandSearchRstPanel::doCustomCommand);
    connect(m_backButton, &DIconButton::clicked, this, &CustomCommandSearchRstPanel::showCustomCommandPanel);
    // 字体颜色随主题变化变化
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &CustomCommandSearchRstPanel::handleThemeTypeChanged);

    connect(m_rebackButton, &IconButton::focusOut, this, &CustomCommandSearchRstPanel::handleIconButtonFocusOut);

    connect(m_cmdListWidget, &ListView::focusOut, this, &CustomCommandSearchRstPanel::handleListViewFocusOut);
}

inline void CustomCommandSearchRstPanel::handleThemeTypeChanged(DGuiApplicationHelper::ColorType themeType)
{
    // qCDebug(customcommand) << "Enter handleThemeTypeChanged";
    DPalette palette = m_label->palette();
    QColor color;
    if (DGuiApplicationHelper::DarkType == themeType) {
        // qCDebug(customcommand) << "Dark theme type in handleThemeTypeChanged";
        color = QColor::fromRgb(192, 198, 212, 102);
    } else {
        // qCDebug(customcommand) << "Light theme type in handleThemeTypeChanged";
        color = QColor::fromRgb(85, 85, 85, 102);
    }

    palette.setBrush(QPalette::Text, color);
    m_label->setPalette(palette);
}

inline void CustomCommandSearchRstPanel::handleIconButtonFocusOut(Qt::FocusReason type)
{
    // qCDebug(customcommand) << "Enter handleIconButtonFocusOut";
    // 焦点切出，没值的时候
    if (Qt::TabFocusReason == type && 0 == m_cmdListWidget->count()) {
        qCDebug(customcommand) << "TabFocusReason and cmdListWidget count is 0";
        // tab 进入 +
        QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_Tab, Qt::MetaModifier);
        QApplication::sendEvent(Utils::getMainWindow(this), &keyPress);
        qCInfo(customcommand) << "search panel focus to '+'";
    }
}

inline void CustomCommandSearchRstPanel::handleListViewFocusOut(Qt::FocusReason type)
{
    // qCDebug(customcommand) << "Enter handleListViewFocusOut";
    Q_UNUSED(type);
    if (Qt::TabFocusReason == type) {
        qCDebug(customcommand) << "TabFocusReason";
        // tab 进入 +
        QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_Tab, Qt::MetaModifier);
        QApplication::sendEvent(Utils::getMainWindow(this), &keyPress);
        qCInfo(customcommand) << "search panel focus on '+'";
        m_cmdListWidget->clearIndex();
    } else if (Qt::BacktabFocusReason == type || Qt::NoFocusReason == type) {
        qCDebug(customcommand) << "BacktabFocusReason or NoFocusReason";
        // shift + tab 返回 返回键               // 列表为空，也返回到返回键上
        m_rebackButton->setFocus();
        m_cmdListWidget->clearIndex();
        qCInfo(customcommand) << "search panel type (" << type << ")";
    }
}

void CustomCommandSearchRstPanel::setSearchFilter(const QString &filter)
{
    qCDebug(customcommand) << "Setting search filter:" << filter;
    m_strFilter = filter;
    QString showText = Utils::getElidedText(m_label->font(), filter, ITEMMAXWIDTH, Qt::ElideMiddle);
    m_label->setText(QString("%1：%2").arg(tr("Search"), showText));
    qCDebug(customcommand) << "Search filter set to:" << showText;
}

void CustomCommandSearchRstPanel::refreshData()
{
    qCDebug(customcommand) << "Refreshing data with current filter:" << m_strFilter;
    ShortcutManager::instance()->fillCommandListData(m_cmdListWidget, m_strFilter);
    qCDebug(customcommand) << "Data refreshed, item count:" << m_cmdListWidget->count();
}

void CustomCommandSearchRstPanel::refreshData(const QString &strFilter)
{
    qCDebug(customcommand) << "Refreshing data with new filter:" << strFilter;
    setSearchFilter(strFilter);
    ShortcutManager::instance()->fillCommandListData(m_cmdListWidget, strFilter);
    qCDebug(customcommand) << "Data refreshed with new filter, item count:" << m_cmdListWidget->count();
}

void CustomCommandSearchRstPanel::doCustomCommand(const QString &strKey)
{
    qCInfo(customcommand) << "Executing custom command with key:" << strKey;
    QAction *item = ShortcutManager::instance()->findActionByKey(strKey);
    QString strCommand = item ? item->data().toString() : "";
    if (!strCommand.endsWith('\n')) {
        qCDebug(customcommand) << "Command does not end with newline, appending one";
        strCommand.append('\n');
    }

    qCInfo(customcommand) << "Executing command:" << strCommand;
    emit handleCustomCurCommand(strCommand);
    emit focusOut();
    qCDebug(customcommand) << "Command execution complete";
}

