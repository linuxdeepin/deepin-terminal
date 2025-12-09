// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tabrenamedlg.h"
#include "utils.h"

#include <DFontSizeManager>
#include <DGuiApplicationHelper>

#include <QEvent>
#include <QKeyEvent>
#include <QTimer>
#include <QDebug>

TabRenameDlg::TabRenameDlg(QWidget *parent) :  DAbstractDialog(parent)
{
    initUi();
    initContentWidget();
    initConnections();
}

void TabRenameDlg::initUi()
{
    setWindowModality(Qt::ApplicationModal);
    // 设置最小值 => 以免一开始挤在一起
    setMinimumWidth(SETTING_DIALOG_WIDTH);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *titleLayout = new QHBoxLayout();
    titleLayout->setSpacing(0);
    titleLayout->setContentsMargins(10, 0, 0, 0);

    m_titleBar = new QWidget(this);
    m_titleBar->setObjectName("titleBar");//Add by ut001000 renfeixiang 2020-08-13
    m_titleBar->setFixedHeight(50);
    m_titleBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_titleBar->setLayout(titleLayout);

    m_logoIcon = new DLabel(this);
    m_logoIcon->setObjectName("logoIcon");//Add by ut001000 renfeixiang 2020-08-13
    m_logoIcon->setFixedSize(QSize(32, 32));
    m_logoIcon->setFocusPolicy(Qt::NoFocus);
    m_logoIcon->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_logoIcon->setPixmap(QIcon::fromTheme("deepin-terminal").pixmap(QSize(32, 32)));

    m_closeButton = new DWindowCloseButton(this);
    m_closeButton->setObjectName("closeButton");//Add by ut001000 renfeixiang 2020-08-13
    m_closeButton->setFocusPolicy(Qt::TabFocus);
    m_closeButton->setIconSize(QSize(50, 50));

    m_titleText = new DLabel(this);
    m_titleText->setObjectName("titleText");//Add by ut001000 renfeixiang 2020-08-13
    m_titleText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_titleText->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(m_titleText, DFontSizeManager::T6);

    titleLayout->addWidget(m_logoIcon, 0, Qt::AlignLeft | Qt::AlignVCenter);
    titleLayout->addWidget(m_titleText);
    titleLayout->addWidget(m_closeButton, 0, Qt::AlignRight | Qt::AlignVCenter);

    //Dialog content
    m_contentLayout = new QVBoxLayout();
    m_contentLayout->setObjectName("contentLayout");//Add by ut001000 renfeixiang 2020-08-13
    m_contentLayout->setSpacing(0);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);

    m_content = new QWidget(this);
    m_content->setObjectName("content");//Add by ut001000 renfeixiang 2020-08-13
    m_content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_content->setLayout(m_contentLayout);

    mainLayout->addWidget(m_titleBar);
    mainLayout->addWidget(m_content);
    setLayout(mainLayout);
}

void TabRenameDlg::addContent(QWidget *content)
{
    Q_ASSERT(nullptr != m_contentLayout);

    m_contentLayout->addWidget(content);
}

QLayout *TabRenameDlg::getContentLayout()
{
    return m_contentLayout;
}

void TabRenameDlg::setText(const QString &tabTitleFormat, const QString &remoteTabTitleFormat)
{
    setNormalLineEditText(tabTitleFormat);
    setRemoteLineEditText(remoteTabTitleFormat);
}

void TabRenameDlg::setNormalLineEditText(const QString &text)
{
    m_tabTitleEdit->getInputedit()->setText(text);
}

void TabRenameDlg::setRemoteLineEditText(const QString &text)
{
    m_remoteTabTitleEdit->getInputedit()->setText(text);
}


QLineEdit *TabRenameDlg::getRemoteTabTitleEdit() const
{
    return m_remoteTabTitleEdit->getInputedit()->lineEdit();
}

void TabRenameDlg::setFocusOnEdit(bool isRemote)
{
    QLineEdit *lineEdit = nullptr;
    if (isRemote) {
        // 连接远程时，设置需要操作的输入框为远程输入框
        lineEdit = getRemoteTabTitleEdit();
    } else {
        // 设置需要操作的输入框为非远程输入框
        lineEdit = getTabTitleEdit();
    }

    // 设置焦点，全选输入框的内容
    lineEdit->setFocus();
    lineEdit->selectAll();
}

QLineEdit *TabRenameDlg::getTabTitleEdit() const
{
    return m_tabTitleEdit->getInputedit()->lineEdit();
}

void TabRenameDlg::initContentWidget()
{
    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setObjectName("mainLayout");//Add by ut001000 renfeixiang 2020-08-13
    m_mainLayout->setContentsMargins(10, 0, 10, 10);

    QWidget *contentwidget = new QWidget(this);
    contentwidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    initTitleLabel();
    initRenameWidget(false);
    initRenameWidget(true);
    initButtonWidget();

    // 添加控件
    m_mainLayout->setSpacing(10);
    m_mainLayout->addWidget(m_titlelabel);

    QVBoxLayout *contentLayout = new QVBoxLayout();
    contentLayout->setSpacing(10);
    contentLayout->setContentsMargins(20, 0, 20, 10);
    contentLayout->addWidget(m_normalWidget);
    contentLayout->addWidget(m_remoteWidget);
    m_mainLayout->addLayout(contentLayout);

    m_mainLayout->addWidget(m_buttonWidget);

    contentwidget->setLayout(m_mainLayout);
    addContent(contentwidget);
    // 设置确认按钮为enter后响应的默认按钮
    // 放在布局后生效，放在布局完成前不生效
    m_confirmButton->setDefault(true);
}

void TabRenameDlg::initConnections()
{
    connect(m_cancelButton, &DPushButton::clicked, this, [ = ] {
        reject();
        close();
    });

    connect(m_confirmButton, &DSuggestButton::clicked, this, [ = ] {
        qInfo() << "confirm rename title";
        QString tabTitleFormat = getTabTitleEdit()->text();
        QString remoteTabTitleFormat = getRemoteTabTitleEdit()->text();
        emit tabTitleFormatRename(tabTitleFormat, remoteTabTitleFormat);
        close();
    });

    connect(m_closeButton, &DPushButton::clicked, this, [ = ] {
        close();
    });

#ifdef DTKWIDGET_CLASS_DSizeMode
    updateSizeMode();
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, &TabRenameDlg::updateSizeMode);
    // 字体变更时进行重新布局
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::fontChanged, this, [this](){
        if (isVisible() && layout()) {
            layout()->invalidate();
            // 根据新界面布局，刷新界面大小
            updateGeometry();
            QTimer::singleShot(0, this, [=](){ resize(SETTING_DIALOG_WIDTH, minimumSizeHint().height()); });
        }
    });
#endif
}

void TabRenameDlg::initTitleLabel()
{
    m_titlelabel = new DLabel(QObject::tr("Rename title"), this);
    m_titlelabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_titlelabel->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(m_titlelabel, DFontSizeManager::T6);
}

void TabRenameDlg::initRenameWidget(bool isRemote)
{
    QHBoxLayout *TAbLayout = new QHBoxLayout();
    TAbLayout->setContentsMargins(0, 0, 0, 0);

    TabRenameWidget *renameWidget = new TabRenameWidget(isRemote);
    TAbLayout->addWidget(renameWidget);

    if (!isRemote) {
        m_normalWidget = new QWidget;
        m_normalWidget->setLayout(TAbLayout);
        m_tabTitleEdit = renameWidget;
    } else {
        m_remoteWidget = new QWidget;
        m_remoteWidget->setLayout(TAbLayout);
        m_remoteTabTitleEdit = renameWidget;
    }
}

void TabRenameDlg::initButtonWidget()
{
    m_buttonWidget = new QWidget;

    QHBoxLayout *buttonTAbLayout = new QHBoxLayout;
    buttonTAbLayout->setContentsMargins(0, 0, 0, 0);
    buttonTAbLayout->setSpacing(10);

    m_cancelButton = new DPushButton(QObject::tr("Cancel", "button"));
    Utils::setSpaceInWord(m_cancelButton);
    DFontSizeManager::instance()->bind(m_cancelButton, DFontSizeManager::T6);

    m_confirmButton = new DSuggestButton(QObject::tr("Confirm", "button"));
    Utils::setSpaceInWord(m_confirmButton);
    DFontSizeManager::instance()->bind(m_confirmButton, DFontSizeManager::T6);

    m_verticalLine = new DVerticalLine;
    m_verticalLine->setFixedSize(1, 28);

    buttonTAbLayout->addWidget(m_cancelButton);
    buttonTAbLayout->addWidget(m_verticalLine);
    buttonTAbLayout->addWidget(m_confirmButton);

    m_buttonWidget->setLayout(buttonTAbLayout);
}

/**
 * @brief 接收 DGuiApplicationHelper::sizeModeChanged() 信号, 根据不同的布局模式调整
 *      当前界面的布局. 只能在界面创建完成后调用.
 */
void TabRenameDlg::updateSizeMode()
{
#ifdef DTKWIDGET_CLASS_DSizeMode
    if (DGuiApplicationHelper::isCompactMode()) {
        m_titleBar->setFixedHeight(WIN_TITLE_BAR_HEIGHT_COMPACT);
        m_logoIcon->setPixmap(QIcon::fromTheme("deepin-terminal").pixmap(QSize(ICON_CTX_SIZE_24, ICON_CTX_SIZE_24)));
        m_closeButton->setIconSize(QSize(ICONSIZE_40_COMPACT, ICONSIZE_40_COMPACT));
        m_verticalLine->setFixedSize(1, VERTICAL_HEIGHT_COMPACT);

    } else {
        m_titleBar->setFixedHeight(WIN_TITLE_BAR_HEIGHT);
        m_logoIcon->setPixmap(QIcon::fromTheme("deepin-terminal").pixmap(QSize(ICON_CTX_SIZE_32, ICON_CTX_SIZE_32)));
        m_closeButton->setIconSize(QSize(ICONSIZE_50, ICONSIZE_50));
        m_verticalLine->setFixedSize(1, VERTICAL_HEIGHT);
    }

    if (layout()) {
        layout()->invalidate();
    }
    // 根据新界面布局，刷新界面大小
    updateGeometry();
    QTimer::singleShot(0, this, [=](){ resize(SETTING_DIALOG_WIDTH, minimumSizeHint().height()); });
#endif
}
