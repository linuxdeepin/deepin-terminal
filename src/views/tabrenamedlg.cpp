// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
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
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(views)
TabRenameDlg::TabRenameDlg(QWidget *parent) :  DAbstractDialog(parent)
{
    qCDebug(views) << "TabRenameDlg constructor enter";
    initUi();
    initContentWidget();
    initConnections();
    qCDebug(views) << "TabRenameDlg constructor exit";
}

void TabRenameDlg::initUi()
{
    qCDebug(views) << "TabRenameDlg::initUi enter";

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
    qCDebug(views) << "Enter TabRenameDlg::addContent";
    Q_ASSERT(nullptr != m_contentLayout);

    qCDebug(views) << "Branch: Adding content widget to layout";
    m_contentLayout->addWidget(content);
}

QLayout *TabRenameDlg::getContentLayout()
{
    // qCDebug(views) << "Enter TabRenameDlg::getContentLayout";
    return m_contentLayout;
}

void TabRenameDlg::setText(const QString &tabTitleFormat, const QString &remoteTabTitleFormat)
{
    // qCDebug(views) << "Enter TabRenameDlg::setText";
    setNormalLineEditText(tabTitleFormat);
    setRemoteLineEditText(remoteTabTitleFormat);
}

void TabRenameDlg::setNormalLineEditText(const QString &text)
{
    qCDebug(views) << "TabRenameDlg::setNormalLineEditText:" << text;

    m_tabTitleEdit->getInputedit()->setText(text);
}

void TabRenameDlg::setRemoteLineEditText(const QString &text)
{
    qCDebug(views) << "TabRenameDlg::setRemoteLineEditText:" << text;

    m_remoteTabTitleEdit->getInputedit()->setText(text);
}


QLineEdit *TabRenameDlg::getRemoteTabTitleEdit() const
{
    // qCDebug(views) << "Enter TabRenameDlg::getRemoteTabTitleEdit";
    return m_remoteTabTitleEdit->getInputedit()->lineEdit();
}

void TabRenameDlg::setFocusOnEdit(bool isRemote)
{
    qCDebug(views) << "TabRenameDlg::setFocusOnEdit isRemote:" << isRemote;

    QLineEdit *lineEdit = nullptr;
    if (isRemote) {
        qCDebug(views) << "Branch: Remote mode, using remote tab title edit";
        // 连接远程时，设置需要操作的输入框为远程输入框
        lineEdit = getRemoteTabTitleEdit();
    } else {
        qCDebug(views) << "Branch: Normal mode, using normal tab title edit";
        // 设置需要操作的输入框为非远程输入框
        lineEdit = getTabTitleEdit();
    }

    qCDebug(views) << "Branch: Setting focus and selecting all text";
    // 设置焦点，全选输入框的内容
    lineEdit->setFocus();
    lineEdit->selectAll();
}

QLineEdit *TabRenameDlg::getTabTitleEdit() const
{
    // qCDebug(views) << "Enter TabRenameDlg::getTabTitleEdit";
    return m_tabTitleEdit->getInputedit()->lineEdit();
}

void TabRenameDlg::initContentWidget()
{
    qCDebug(views) << "TabRenameDlg::initContentWidget enter";

    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setObjectName("mainLayout");//Add by ut001000 renfeixiang 2020-08-13
    m_mainLayout->setContentsMargins(10, 0, 10, 10);

    qCDebug(views) << "Branch: Creating content widget";
    QWidget *contentwidget = new QWidget(this);
    contentwidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    qCDebug(views) << "Branch: Initializing UI components";
    initTitleLabel();
    initRenameWidget(false);
    initRenameWidget(true);
    initButtonWidget();

    qCDebug(views) << "Branch: Adding widgets to layout";
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

    qCDebug(views) << "Branch: Setting up content widget layout";
    contentwidget->setLayout(m_mainLayout);
    addContent(contentwidget);
    // 设置确认按钮为enter后响应的默认按钮
    // 放在布局后生效，放在布局完成前不生效
    m_confirmButton->setDefault(true);
}

void TabRenameDlg::initConnections()
{
    qCDebug(views) << "TabRenameDlg::initConnections enter";

    connect(m_cancelButton, &DPushButton::clicked, this, [ = ] {
        qCDebug(views) << "Lambda: Cancel button clicked";
        reject();
        close();
    });

    qCDebug(views) << "Branch: Connecting confirm button";
    connect(m_confirmButton, &DSuggestButton::clicked, this, [ = ] {
        qCInfo(views) << "confirm rename title";
        QString tabTitleFormat = getTabTitleEdit()->text();
        QString remoteTabTitleFormat = getRemoteTabTitleEdit()->text();
        emit tabTitleFormatRename(tabTitleFormat, remoteTabTitleFormat);
        close();
    });

    qCDebug(views) << "Branch: Connecting close button";
    connect(m_closeButton, &DPushButton::clicked, this, [ = ] {
        qCDebug(views) << "Lambda: Close button clicked";
        close();
    });

#ifdef DTKWIDGET_CLASS_DSizeMode
    qCDebug(views) << "Branch: Setting up size mode handling";
    updateSizeMode();
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, &TabRenameDlg::updateSizeMode);
    // 字体变更时进行重新布局
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::fontChanged, this, [this](){
        if (isVisible() && layout()) {
            qCDebug(views) << "Branch: Dialog is visible and has layout, updating";
            layout()->invalidate();
            // 根据新界面布局，刷新界面大小
            updateGeometry();
            QTimer::singleShot(0, this, [=](){ resize(SETTING_DIALOG_WIDTH, minimumSizeHint().height()); });
        }
    });
#else
    qCDebug(views) << "Branch: Size mode handling not available";
#endif
}

void TabRenameDlg::initTitleLabel()
{
    qCDebug(views) << "Enter TabRenameDlg::initTitleLabel";
    m_titlelabel = new DLabel(QObject::tr("Rename title"), this);
    m_titlelabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_titlelabel->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(m_titlelabel, DFontSizeManager::T6);
}

void TabRenameDlg::initRenameWidget(bool isRemote)
{
    qCDebug(views) << "Enter TabRenameDlg::initRenameWidget with isRemote:" << isRemote;
    QHBoxLayout *TAbLayout = new QHBoxLayout();
    TAbLayout->setContentsMargins(0, 0, 0, 0);

    qCDebug(views) << "Branch: Creating rename widget";
    TabRenameWidget *renameWidget = new TabRenameWidget(isRemote);
    TAbLayout->addWidget(renameWidget);

    if (!isRemote) {
        qCDebug(views) << "Branch: Creating normal widget";
        m_normalWidget = new QWidget;
        m_normalWidget->setLayout(TAbLayout);
        m_tabTitleEdit = renameWidget;
    } else {
        qCDebug(views) << "Branch: Creating remote widget";
        m_remoteWidget = new QWidget;
        m_remoteWidget->setLayout(TAbLayout);
        m_remoteTabTitleEdit = renameWidget;
    }
}

void TabRenameDlg::initButtonWidget()
{
    qCDebug(views) << "Enter TabRenameDlg::initButtonWidget";
    m_buttonWidget = new QWidget;

    qCDebug(views) << "Branch: Creating button layout";
    QHBoxLayout *buttonTAbLayout = new QHBoxLayout;
    buttonTAbLayout->setContentsMargins(0, 0, 0, 0);
    buttonTAbLayout->setSpacing(10);

    qCDebug(views) << "Branch: Creating cancel button";
    m_cancelButton = new DPushButton(QObject::tr("Cancel", "button"));
    Utils::setSpaceInWord(m_cancelButton);
    DFontSizeManager::instance()->bind(m_cancelButton, DFontSizeManager::T6);

    qCDebug(views) << "Branch: Creating confirm button";
    m_confirmButton = new DSuggestButton(QObject::tr("Confirm", "button"));
    Utils::setSpaceInWord(m_confirmButton);
    DFontSizeManager::instance()->bind(m_confirmButton, DFontSizeManager::T6);

    qCDebug(views) << "Branch: Creating vertical line";
    m_verticalLine = new DVerticalLine;
    m_verticalLine->setFixedSize(1, 28);

    qCDebug(views) << "Branch: Adding buttons to layout";
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
    qCDebug(views) << "Enter TabRenameDlg::updateSizeMode";

#ifdef DTKWIDGET_CLASS_DSizeMode
    qCDebug(views) << "Branch: Size mode handling available";

    if (DGuiApplicationHelper::isCompactMode()) {
        qCDebug(views) << "Branch: Compact mode detected, applying compact layout";
        m_titleBar->setFixedHeight(WIN_TITLE_BAR_HEIGHT_COMPACT);
        m_logoIcon->setPixmap(QIcon::fromTheme("deepin-terminal").pixmap(QSize(ICON_CTX_SIZE_24, ICON_CTX_SIZE_24)));
        m_closeButton->setIconSize(QSize(ICONSIZE_40_COMPACT, ICONSIZE_40_COMPACT));
        m_verticalLine->setFixedSize(1, VERTICAL_HEIGHT_COMPACT);

    } else {
        qCDebug(views) << "Branch: Normal mode detected, applying normal layout";
        m_titleBar->setFixedHeight(WIN_TITLE_BAR_HEIGHT);
        m_logoIcon->setPixmap(QIcon::fromTheme("deepin-terminal").pixmap(QSize(ICON_CTX_SIZE_32, ICON_CTX_SIZE_32)));
        m_closeButton->setIconSize(QSize(ICONSIZE_50, ICONSIZE_50));
        m_verticalLine->setFixedSize(1, VERTICAL_HEIGHT);
    }

    if (layout()) {
        qCDebug(views) << "Branch: Layout exists, invalidating and updating";
        layout()->invalidate();
    }

    qCDebug(views) << "Branch: Updating geometry";
    // 根据新界面布局，刷新界面大小
    updateGeometry();
    QTimer::singleShot(0, this, [=](){ resize(SETTING_DIALOG_WIDTH, minimumSizeHint().height()); });
#else
    qCDebug(views) << "Branch: Size mode handling not available";
#endif
}
