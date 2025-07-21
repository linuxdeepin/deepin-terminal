// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tabrenamewidget.h"

//dtk
#include <DFontSizeManager>

//qt
#include <QDebug>
#include <QKeyEvent>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(views)
TabRenameWidget::TabRenameWidget(bool isRemote, bool isSetting, QWidget *parent)
    : QWidget(parent)
    , m_isRemote(isRemote)
    , m_isSetting(isSetting)
{
    qCDebug(views) << "TabRenameWidget constructor enter, isRemote:" << isRemote << "isSetting:" << isSetting;
    initChoseMenu();
    initUi();
    initConnections();
    qCDebug(views) << "TabRenameWidget constructor exit";
}

void TabRenameWidget::initUi()
{
    qCDebug(views) << "TabRenameWidget::initUi enter";

    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    // 设置布局之间的宽度,之前的间距比较宽
    m_layout->setSpacing(10);
    // widet 自己设置0 0 0 0
    this->setContentsMargins(0, 0, 0, 0);

    // 内容输入框
    m_inputedit = new DLineEdit(this);
    m_inputedit->setText("%n:%d");
    m_inputedit->setMaximumWidth(172);
    m_inputedit->setMinimumWidth(135);
    DFontSizeManager::instance()->bind(m_inputedit, DFontSizeManager::T6);

    // 插入按钮
    m_choseButton = new DPushButton(tr("Insert"), this);
    // 添加下拉菜单
    m_choseButton->setMenu(m_choseMenu);
    m_choseButton->setAutoDefault(false);
    m_choseButton->setDefault(false);

    qCDebug(views) << "Branch: Checking if setting mode";
    //  设置界面隐藏clearbutton并且不需要初始化标题 重命名窗口不用隐藏clearbutton需要初始化标题
    if (!m_isSetting) {
        qCDebug(views) << "Branch: Not in setting mode, initializing label";
        initLabel();
    } else {
        qCDebug(views) << "Branch: In setting mode, disabling clear button";
        m_inputedit->lineEdit()->setClearButtonEnabled(false);
    }

    qCDebug(views) << "Branch: Adding widgets to layout";
    m_layout->addWidget(m_inputedit);
    m_layout->addWidget(m_choseButton);
    qCDebug(views) << "TabRenameWidget::initUi finished";
}

void TabRenameWidget::initChoseMenu()
{
    qCDebug(views) << "TabRenameWidget::initChoseMenu enter";

    m_choseMenu = new DMenu(this);
    DFontSizeManager::instance()->bind(m_choseMenu, DFontSizeManager::T6);
    if (m_isRemote) {
        qCDebug(views) << "Branch: Remote mode, initializing remote menu";
        // 初始化远程菜单
        initRemoteChoseMenu();
    } else {
        qCDebug(views) << "Branch: Normal mode, initializing normal menu";
        // 初始化正常菜单
        initNormalChoseMenu();
    }
    qCDebug(views) << "TabRenameWidget::initChoseMenu finished";
}

void TabRenameWidget::initRemoteChoseMenu()
{
    qCDebug(views) << "TabRenameWidget::initRemoteChoseMenu enter";

    QStringList list;
    list << tr("username: %u") << tr("username@: %U") << tr("remote host: %h")
         << tr("session number: %#") << tr("title set by shell: %w");

    qCDebug(views) << "Branch: Adding remote menu actions";
    foreach (auto it, list) {
        // qCDebug(views) << "Branch: Adding action:" << it;
        QAction *ac = new QAction(it, m_choseMenu);
        m_choseMenu->addAction(ac);
    }
    qCDebug(views) << "TabRenameWidget::initRemoteChoseMenu finished";
}

void TabRenameWidget::initNormalChoseMenu()
{
    qCDebug(views) << "TabRenameWidget::initNormalChoseMenu enter";

    QStringList list;
    list << tr("program name: %n") << tr("current directory (short): %d")
         << tr("current directory (long): %D") << tr("session number: %#")
         << tr("username: %u") << tr("local host: %h")
         << tr("title set by shell: %w");

    qCDebug(views) << "Branch: Adding normal menu actions";
    foreach (auto it, list) {
        // qCDebug(views) << "Branch: Adding action:" << it;
        QAction *ac = new QAction(it, m_choseMenu);
        m_choseMenu->addAction(ac);
    }
    qCDebug(views) << "TabRenameWidget::initNormalChoseMenu finished";
}

void TabRenameWidget::initConnections()
{
    qCDebug(views) << "TabRenameWidget::initConnections enter";

    connect(m_choseMenu, &DMenu::triggered, this, [ = ](QAction * ac) {
        qCDebug(views) << "TabRenameWidget menu item selected:" << ac->text();
        QStringList spiltlist = ac->text().split("%");
        m_inputedit->lineEdit()->insert("%" + spiltlist.at(1));
        //向输入条中输入内容后，焦点应该同步设置过去
        m_inputedit->lineEdit()->setFocus(Qt::MouseFocusReason);
    });
    qCDebug(views) << "TabRenameWidget::initConnections finished";
}

void TabRenameWidget::initLabel()
{
    qCDebug(views) << "TabRenameWidget::initLabel enter";

    m_Label = new QLabel;
    if (!m_isRemote) {
        qCDebug(views) << "Branch: Setting normal tab title format text";
        m_Label->setText(QObject::tr("Tab title format"));
    } else {
        qCDebug(views) << "Branch: Setting remote tab title format text";
        m_Label->setText(QObject::tr("Remote tab title format"));
    }

    qCDebug(views) << "Branch: Setting label properties";
    DFontSizeManager::instance()->bind(m_Label, DFontSizeManager::T6);

    // label要跟随字体扩展 => 所有控件都扩展效果不好
    m_Label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    qCDebug(views) << "Branch: Adding label to layout";
    m_layout->addWidget(m_Label);
    qCDebug(views) << "TabRenameWidget::initLabel finished";
}

DLineEdit *TabRenameWidget::getInputedit() const
{
    // qCDebug(views) << "Enter TabRenameWidget::getInputedit";
    return m_inputedit;
}


