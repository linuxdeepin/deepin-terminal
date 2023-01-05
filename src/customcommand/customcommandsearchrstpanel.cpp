// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "customcommandsearchrstpanel.h"
#include "customcommandoptdlg.h"
#include "shortcutmanager.h"
#include "iconbutton.h"
#include "listview.h"
#include "utils.h"
#include "mainwindow.h"

#include <DApplicationHelper>
#include <DMessageBox>

#include <QAction>
#include <QKeyEvent>
#include <QApplication>
#include <QCoreApplication>
#include <QTimer>
#include <QDebug>

CustomCommandSearchRstPanel::CustomCommandSearchRstPanel(QWidget *parent)
    : CommonPanel(parent)
{
    Utils::set_Object_Name(this);
    initUI();
}

void CustomCommandSearchRstPanel::initUI()
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);

    m_rebackButton = new IconButton(this);
    m_rebackButton->setObjectName("CustomRebackButton");
    m_backButton = m_rebackButton;
    m_backButton->setIcon(DStyle::StandardPixmap::SP_ArrowLeave);
    m_backButton->setFixedSize(QSize(36, 36));
    m_backButton->setFocusPolicy(Qt::TabFocus);

    m_label = new DLabel(this);
    m_label->setObjectName("CustomSearchFilterlabel");//Add by ut001000 renfeixiang 2020-08-13
    m_label->setAlignment(Qt::AlignCenter);
    // 字体颜色随主题变化变化
    DPalette palette = m_label->palette();
    QColor color;
    if (DApplicationHelper::DarkType == DApplicationHelper::instance()->themeType())
        color = QColor::fromRgb(192, 198, 212, 102);
    else
        color = QColor::fromRgb(85, 85, 85, 102);

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
    hlayout->setMargin(0);

    QVBoxLayout *vlayout = new QVBoxLayout();
    vlayout->addSpacing(10);
    vlayout->addLayout(hlayout);
    vlayout->addWidget(m_cmdListWidget);
    vlayout->setMargin(0);
    vlayout->setSpacing(10);
    setLayout(vlayout);

    connect(m_cmdListWidget, &ListView::itemClicked, this, &CustomCommandSearchRstPanel::doCustomCommand);
    connect(m_backButton, &DIconButton::clicked, this, &CustomCommandSearchRstPanel::showCustomCommandPanel);
    // 字体颜色随主题变化变化
    connect(DApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &CustomCommandSearchRstPanel::handleThemeTypeChanged);

    connect(m_rebackButton, &IconButton::focusOut, this, &CustomCommandSearchRstPanel::handleIconButtonFocusOut);

    connect(m_cmdListWidget, &ListView::focusOut, this, &CustomCommandSearchRstPanel::handleListViewFocusOut);
}

inline void CustomCommandSearchRstPanel::handleThemeTypeChanged(DGuiApplicationHelper::ColorType themeType)
{
    DPalette palette = m_label->palette();
    QColor color;
    if (DApplicationHelper::DarkType == themeType)
        color = QColor::fromRgb(192, 198, 212, 102);
    else
        color = QColor::fromRgb(85, 85, 85, 102);

    palette.setBrush(QPalette::Text, color);
    m_label->setPalette(palette);
}

inline void CustomCommandSearchRstPanel::handleIconButtonFocusOut(Qt::FocusReason type)
{
    // 焦点切出，没值的时候
    if (Qt::TabFocusReason == type && 0 == m_cmdListWidget->count()) {
        // tab 进入 +
        QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_Tab, Qt::MetaModifier);
        QApplication::sendEvent(Utils::getMainWindow(this), &keyPress);
        qInfo() << "search panel focus to '+'";
    }
}

inline void CustomCommandSearchRstPanel::handleListViewFocusOut(Qt::FocusReason type)
{
    Q_UNUSED(type);
    if (Qt::TabFocusReason == type) {
        // tab 进入 +
        QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_Tab, Qt::MetaModifier);
        QApplication::sendEvent(Utils::getMainWindow(this), &keyPress);
        qInfo() << "search panel focus on '+'";
        m_cmdListWidget->clearIndex();
    } else if (Qt::BacktabFocusReason == type || Qt::NoFocusReason == type) {
        // shift + tab 返回 返回键               // 列表为空，也返回到返回键上
        m_rebackButton->setFocus();
        m_cmdListWidget->clearIndex();
        qInfo() << "search panel type" << type;
    }
}

void CustomCommandSearchRstPanel::setSearchFilter(const QString &filter)
{
    m_strFilter = filter;
    QString showText = Utils::getElidedText(m_label->font(), filter, ITEMMAXWIDTH, Qt::ElideMiddle);
    m_label->setText(QString("%1：%2").arg(tr("Search"), showText));
}

void CustomCommandSearchRstPanel::refreshData()
{
    ShortcutManager::instance()->fillCommandListData(m_cmdListWidget, m_strFilter);
}

void CustomCommandSearchRstPanel::refreshData(const QString &strFilter)
{
    setSearchFilter(strFilter);
    ShortcutManager::instance()->fillCommandListData(m_cmdListWidget, strFilter);
}

void CustomCommandSearchRstPanel::doCustomCommand(const QString &strKey)
{
    qInfo() << "doCustomCommand,key=" << strKey;
    QAction *item = ShortcutManager::instance()->findActionByKey(strKey);
    QString strCommand = item ? item->data().toString() : "";
    if (!strCommand.endsWith('\n'))
        strCommand.append('\n');

    emit handleCustomCurCommand(strCommand);
    emit focusOut();
}

