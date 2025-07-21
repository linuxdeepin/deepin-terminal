// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "customcommandoptdlg.h"
#include "termcommandlinkbutton.h"
#include "utils.h"
#include "service.h"

#include <DButtonBox>
#include <DPushButton>
#include <DGroupBox>
#include <DLabel>
#include <DKeySequenceEdit>
#include <DCommandLinkButton>
#include <DDialogCloseButton>
#include <DGuiApplicationHelper>
#include <DGuiApplicationHelper>
#include <DWidgetUtil>
#include <DLog>
#include <DFontSizeManager>

#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QApplication>
#include <QClipboard>

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(customcommand,"org.deepin.terminal.customcommand")
#else
Q_LOGGING_CATEGORY(customcommand,"org.deepin.terminal.customcommand",QtInfoMsg)
#endif

CustomCommandOptDlg::CustomCommandOptDlg(CustomCmdOptType type, CustomCommandData *currItemData, QWidget *parent)
    : DAbstractDialog(parent)
    , m_type(type)
    , m_nameLineEdit(new DLineEdit(this))
    , m_commandLineEdit(new DLineEdit(this))
    , m_shortCutLineEdit(new DKeySequenceEdit(this))
    , m_bDelOpt(false)
{
    qCDebug(customcommand) << "Creating CustomCommandOptDlg, type:" << type;
    /******** Add by ut001000 renfeixiang 2020-08-13:增加 Begin***************/
    Utils::set_Object_Name(this);
    m_nameLineEdit->setObjectName("CustomNameLineEdit");
    m_commandLineEdit->setObjectName("CustomCommandLineEdit");
    m_shortCutLineEdit->setObjectName("CustomShortCutLineEdit");
    /******** Add by ut001000 renfeixiang 2020-08-13:增加 End***************/
    setWindowModality(Qt::WindowModal);
    if (currItemData) {
        qCDebug(customcommand) << "Branch: currItemData is not null";
        m_currItemData = new CustomCommandData;
        *m_currItemData = *currItemData;
    }

    initUITitle();
    initTitleConnections();
    initUI();

    connect(Service::instance(), &Service::refreshCommandPanel, this, &CustomCommandOptDlg::slotRefreshData);
}

CustomCommandOptDlg::~CustomCommandOptDlg()
{
    qCDebug(customcommand) << "Destroying CustomCommandOptDlg";
    if (m_currItemData) {
        qCDebug(customcommand) << "m_currItemData is not null, deleting";
        delete m_currItemData;
        m_currItemData = nullptr;
    }
    if (m_newAction) {
        qCDebug(customcommand) << "m_newAction is not null, deleting";
        delete  m_newAction;
        m_newAction = nullptr;
    }
}

void CustomCommandOptDlg::initUI()
{
    qCDebug(customcommand) << "Initializing CustomCommandOptDlg UI";
    QWidget *contentFrame = new QWidget(this);

    QVBoxLayout *contentLayout = new QVBoxLayout;
    contentLayout->setSpacing(m_iSpaceSizeTen);
    contentLayout->setContentsMargins(m_iLayoutSizeZero, m_iLayoutSizeZero, m_iLayoutSizeZero, m_iLayoutSizeZero);

    QWidget *nameFrame = new QWidget(this);
    nameFrame->setFixedWidth(m_iFixedWidth);
    QHBoxLayout *nameLayout = new QHBoxLayout;
    nameLayout->setSpacing(m_iSpaceSizeZero);
    nameLayout->setContentsMargins(m_iLayoutLeftSize, m_iLayoutSizeZero, m_iLayoutRightSize, m_iLayoutSizeZero);
    nameFrame->setLayout(nameLayout);

    QWidget *cmdFrame = new QWidget(this);
    QHBoxLayout *cmdLayout = new QHBoxLayout;
    cmdLayout->setSpacing(m_iSpaceSizeZero);
    cmdLayout->setContentsMargins(m_iLayoutLeftSize, m_iLayoutSizeZero, m_iLayoutRightSize, m_iLayoutSizeZero);
    cmdFrame->setLayout(cmdLayout);

    QWidget *scFrame = new QWidget(this);
    QHBoxLayout *scLayout = new QHBoxLayout;
    scLayout->setSpacing(m_iSpaceSizeZero);
    scLayout->setContentsMargins(m_iLayoutLeftSize, m_iLayoutSizeZero, m_iLayoutRightSize, m_iLayoutSizeZero);
    scFrame->setLayout(scLayout);

    DLabel *nameLabel = new DLabel(tr("Name:"));

    DLabel *cmdLabel = new DLabel(tr("Command:"));

    DLabel *shortCutLabel = new DLabel(tr("Shortcuts:"));

    m_nameLineEdit->setFixedWidth(m_iLineEditWidth);
    m_commandLineEdit->setFixedWidth(m_iLineEditWidth);
    m_shortCutLineEdit->setFixedWidth(m_iLineEditWidth);
    m_shortCutLineEdit->setFocusPolicy(Qt::FocusPolicy::StrongFocus);

    QTimer::singleShot(m_iSingleShotTime, this, [&]() {
        m_nameLineEdit->lineEdit()->selectAll();
    });

    m_nameLineEdit->lineEdit()->setPlaceholderText(tr("Required"));
    m_commandLineEdit->lineEdit()->setPlaceholderText(tr("Required"));
    m_shortCutLineEdit->ShortcutDirection(Qt::AlignLeft);

    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(m_nameLineEdit);

    cmdLayout->addWidget(cmdLabel);
    cmdLayout->addWidget(m_commandLineEdit);

    scLayout->addWidget(shortCutLabel);
    scLayout->addWidget(m_shortCutLineEdit);

    contentLayout->addWidget(nameFrame);
    contentLayout->addWidget(cmdFrame);
    contentLayout->addWidget(scFrame);

    contentFrame->setLayout(contentLayout);

    connect(m_nameLineEdit, &DLineEdit::editingFinished, this, &CustomCommandOptDlg::slotNameLineEditingFinished);

    connect(m_commandLineEdit, &DLineEdit::editingFinished, this, &CustomCommandOptDlg::slotCommandLineEditingFinished);

    addContent(contentFrame);
    //判断是添加操作窗口还是修改操作窗口
    if (CCT_ADD == m_type) {
        qCInfo(customcommand) << "Setting up Add Command dialog";
        setTitle(tr("Add Command"));
        initCommandFromClipBoardText();

#ifdef DTKWIDGET_CLASS_DSizeMode
        getMainLayout()->addSpacing(m_iSpaceSizeSeven);
#else
        getMainLayout()->addSpacing(m_iSpaceSizeEighteen);
#endif
    } else {
        qCInfo(customcommand) << "Setting up Edit Command dialog";
        setTitle(tr("Edit Command"));

        m_deleteCmdWidget = new QWidget(this);
        m_deleteCmdWidget->setFixedHeight(m_iFixedHeight);

        QHBoxLayout *deleteCmdLayout = new QHBoxLayout();
        deleteCmdLayout->setSpacing(m_iSpaceSizeZero);
        deleteCmdLayout->setContentsMargins(m_iLayoutSizeZero, m_iLayoutSizeZero, m_iLayoutSizeZero, m_iLayoutSizeZero);
        TermCommandLinkButton *deleteCmdBtn = new TermCommandLinkButton();
        deleteCmdBtn->setFocusPolicy(Qt::TabFocus);
        deleteCmdBtn->setText(tr("Delete Command"));

        deleteCmdLayout->addStretch();
        deleteCmdLayout->addWidget(deleteCmdBtn);
        deleteCmdLayout->addStretch();
        m_deleteCmdWidget->setLayout(deleteCmdLayout);

        getMainLayout()->addWidget(m_deleteCmdWidget);

        connect(deleteCmdBtn, &DCommandLinkButton::clicked, this, &CustomCommandOptDlg::slotDelCurCustomCommand);

        QString strName = m_currItemData->m_cmdName;
        QString strCommad = m_currItemData->m_cmdText;
        QKeySequence keyseq = QKeySequence(m_currItemData->m_cmdShortcut);
        m_lastCmdShortcut = m_currItemData->m_cmdShortcut;
        m_nameLineEdit->setText(strName);
        m_commandLineEdit->setText(strCommad);
        m_shortCutLineEdit->setKeySequence(keyseq);
    }

    addCancelConfirmButtons();
    setCancelBtnText(tr("Cancel", "button"));
    if (CCT_ADD == m_type) {
        qCDebug(customcommand) << "Setting Add button text";
        setConfirmBtnText(tr("Add", "button"));
    } else {
        qCDebug(customcommand) << "Setting Save button text";
        setConfirmBtnText(tr("Save", "button"));
    }
    m_lastCmdShortcut = m_shortCutLineEdit->keySequence().toString();
    connect(this, &CustomCommandOptDlg::confirmBtnClicked, this, &CustomCommandOptDlg::slotAddSaveButtonClicked);
    connect(m_shortCutLineEdit, &KeySequenceEdit::editingFinished, this, &CustomCommandOptDlg::slotShortCutLineEditingFinished);

#ifdef DTKWIDGET_CLASS_DSizeMode
    updateSizeMode();
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, &CustomCommandOptDlg::updateSizeMode);
    // 字体变更时更新布局
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::fontChanged, this, [this](){
        if (isVisible() && layout()) {
            qCDebug(customcommand) << "isVisible and layout conditions met";
            layout()->invalidate();
            updateGeometry();
            // 根据新界面布局，刷新界面大小
            QTimer::singleShot(0, this, [=](){ resize(SETTING_DIALOG_WIDTH, minimumSizeHint().height()); });
        }
    });

#else
    if (CCT_ADD == m_type) {
        qCDebug(customcommand) << "Setting fixed size for Add type";
        setFixedSize(m_iFixedWidth, m_iFixedHeightAddSize);
    } else {
        qCDebug(customcommand) << "Setting fixed size for Edit type";
        setFixedSize(m_iFixedWidth, m_iFixedHeightEditSize);
    }
#endif
}

inline void CustomCommandOptDlg::slotNameLineEditingFinished()
{
    qCDebug(customcommand) << "Enter slotNameLineEditingFinished";
    if (m_nameLineEdit->text().isEmpty()) {
        qCDebug(customcommand) << "name line edit is empty";
        m_nameLineEdit->lineEdit()->setPlaceholderText(tr("Required"));
    }
}

inline void CustomCommandOptDlg::slotCommandLineEditingFinished()
{
    qCDebug(customcommand) << "Enter slotCommandLineEditingFinished";
    if (m_commandLineEdit->text().isEmpty()) {
        qCDebug(customcommand) << "command line edit is empty";
        m_commandLineEdit->lineEdit()->setPlaceholderText(tr("Required"));
    }
}

inline void CustomCommandOptDlg::slotShortCutLineEditingFinished(const QKeySequence &sequence)
{
    qCDebug(customcommand) << "Enter slotShortCutLineEditingFinished";
    //删除
    if ("Backspace" == sequence.toString()) {
        qCInfo(customcommand) << "The KeySequenceE is Backspace";
        m_shortCutLineEdit->clear();
        m_lastCmdShortcut = "";
        return;
    }
    // 取消
    if ("Esc" == sequence.toString()) {
        qCInfo(customcommand) << "The KeySequenceE is Esc";
        m_shortCutLineEdit->clear();
        m_shortCutLineEdit->setKeySequence(QKeySequence(m_lastCmdShortcut));
        /***add by ut001121 zhangmeng 20200521 在快捷键编辑框中按下ESC键时退出窗口 修复BUG27554***/
        close();
        return;
    }
    QString checkName;
    if (m_type != CCT_ADD) {
        qCDebug(customcommand) << "type is not CCT_ADD";
        checkName = m_nameLineEdit->text();
    }

    QString reason;
    // 判断快捷键是否冲突
    if (!ShortcutManager::instance()->checkShortcutValid(checkName, sequence.toString(), reason)) {
        qCWarning(customcommand) << "Shortcut key conflict";
        // 冲突
        if (sequence.toString() != "Esc") {
            qCWarning(customcommand) << "The current shortcut key is not Esc! ("<< sequence <<")";
            showShortcutConflictMsgbox(reason);
        }
        m_shortCutLineEdit->clear();
        m_shortCutLineEdit->setKeySequence(QKeySequence(m_lastCmdShortcut));
        return;
    }
    m_lastCmdShortcut = sequence.toString();
}

void CustomCommandOptDlg::initUITitle()
{
    qCDebug(customcommand) << "Enter initUITitle";
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(SPACEHEIGHT);
    mainLayout->setContentsMargins(0, 0, 0, 10);

    QHBoxLayout *titleLayout = new QHBoxLayout();
    titleLayout->setSpacing(0);
    titleLayout->setContentsMargins(0, 0, 0, 0);

    m_titleBar = new QWidget(this);
    m_titleBar->setObjectName("CustomTitleBar");//Add by ut001000 renfeixiang 2020-08-13
    m_titleBar->setFixedHeight(50);
    m_titleBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_titleBar->setLayout(titleLayout);

    m_logoIcon = new DLabel(this);
    m_logoIcon->setObjectName("CustomLogoIcon");//Add by ut001000 renfeixiang 2020-08-13
    m_logoIcon->setFixedSize(QSize(50, 50));
    m_logoIcon->setFocusPolicy(Qt::NoFocus);
    m_logoIcon->setAttribute(Qt::WA_TransparentForMouseEvents);

    m_closeButton = new DWindowCloseButton(this);
    m_closeButton->setObjectName("CustomCloseButton");//Add by ut001000 renfeixiang 2020-08-13
    m_closeButton->setFocusPolicy(Qt::TabFocus);//m_closeButton->setFocusPolicy(Qt::NoFocus);
    m_closeButton->setIconSize(QSize(50, 50));

    m_titleText = new DLabel(this);
    m_titleText->setObjectName("CustomTitleTextLabel");//Add by ut001000 renfeixiang 2020-08-13
    m_titleText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_titleText->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(m_titleText, DFontSizeManager::T5, QFont::DemiBold);
    // 字色
    DPalette palette = m_titleText->palette();
    QColor color;
    if (DGuiApplicationHelper::DarkType == DGuiApplicationHelper::instance()->themeType()) {
        qCDebug(customcommand) << "Dark theme type";
        color = QColor::fromRgb(192, 198, 212, 255);
    } else {
        qCDebug(customcommand) << "Light theme type";
        color = QColor::fromRgb(0, 26, 46, 255);
    }

    palette.setBrush(QPalette::WindowText, color);
    m_titleText->setPalette(palette);

    titleLayout->addWidget(m_logoIcon, 0, Qt::AlignLeft | Qt::AlignVCenter);
    titleLayout->addWidget(m_titleText, 0, Qt::AlignHCenter);
    titleLayout->addWidget(m_closeButton, 0, Qt::AlignRight | Qt::AlignTop);

    //Dialog content
    m_contentLayout = new QVBoxLayout();
    m_contentLayout->setObjectName("CustomContentLayout");//Add by ut001000 renfeixiang 2020-08-13
    m_contentLayout->setSpacing(0);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);

    m_content = new QWidget(this);
    m_content->setObjectName("CustomContentWidget");//Add by ut001000 renfeixiang 2020-08-13
    m_content->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_content->setLayout(m_contentLayout);

    mainLayout->addWidget(m_titleBar, 0, Qt::AlignTop);
    mainLayout->addWidget(m_content);
    setLayout(mainLayout);

    m_mainLayout = mainLayout;
}

void CustomCommandOptDlg::initTitleConnections()
{
    qCDebug(customcommand) << "Enter initTitleConnections";
    connect(m_closeButton, &DWindowCloseButton::clicked, this, &CustomCommandOptDlg::slotCloseButtonClicked);
    // 字体颜色随主题变化变化
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &CustomCommandOptDlg::slotThemeTypeChanged);
}

inline void CustomCommandOptDlg::slotCloseButtonClicked()
{
    // qCDebug(customcommand) << "Enter slotCloseButtonClicked";
    this->close();
}

inline void CustomCommandOptDlg::slotThemeTypeChanged(DGuiApplicationHelper::ColorType themeType)
{
    qCDebug(customcommand) << "Enter slotThemeTypeChanged";
    DPalette palette = m_titleText->palette();
    QColor color;
    if (DGuiApplicationHelper::DarkType == themeType) {
        qCDebug(customcommand) << "Dark theme type in slotThemeTypeChanged";
        color = QColor::fromRgb(192, 198, 212, 255);
    } else {
        qCDebug(customcommand) << "Light theme type in slotThemeTypeChanged";
        color = QColor::fromRgb(0, 26, 46, 255);
    }

    palette.setBrush(QPalette::WindowText, color);
    m_titleText->setPalette(palette);
}

void CustomCommandOptDlg::initCommandFromClipBoardText()
{
    qCDebug(customcommand) << "Initializing command from clipboard";
    if (m_commandLineEdit) {
        qCDebug(customcommand) << "m_commandLineEdit is not null";
        MainWindow *main = Utils::getMainWindow(this);//;getMainWindow();
        if (main != nullptr) {
            qCDebug(customcommand) << "main window is not null";
            QString clipText = main->selectedText();
            qCDebug(customcommand) << "Clipboard text:" << clipText;
            m_commandLineEdit->setText(clipText.trimmed());
        }
    }
}

QAction *CustomCommandOptDlg::getCurCustomCmd()
{
    // qCDebug(customcommand) << "Enter getCurCustomCmd";
    return m_newAction;
}

bool CustomCommandOptDlg::isDelCurCommand()
{
    // qCDebug(customcommand) << "Enter isDelCurCommand";
    return m_bDelOpt;
}

void CustomCommandOptDlg::slotAddSaveButtonClicked()
{
    qCDebug(customcommand) << "Enter slotAddSaveButtonClicked";
    QString strName = m_nameLineEdit->text();
    QString strCommand = m_commandLineEdit->text();
    QKeySequence keytmp = m_shortCutLineEdit->keySequence();

    strName = strName.trimmed();//空格的名称是无效的，剔除名称前后的空格
    if (strName.isEmpty()) {
        qCWarning(customcommand) << "The name of the user-defined command is empty!";
        m_nameLineEdit->showAlertMessage(tr("Please enter a name"), m_nameLineEdit->parentWidget());
        return;
    }

    /***add begin by ut001121 zhangmeng 20200615 限制名称字符长度 修复BUG31286***/
    if (strName.length() > MAX_NAME_LEN) {
        qCWarning(customcommand) << "The name should be no more than 32 characters";
        m_nameLineEdit->showAlertMessage(QObject::tr("The name should be no more than 32 characters"), m_nameLineEdit->parentWidget());
        return;
    }
    /***add end by ut001121***/
    QString strCommandtemp = strCommand.trimmed();//空格的命令是无效的
    if (strCommandtemp.isEmpty()) {
        qCWarning(customcommand) << "The custom command is empty";
        m_commandLineEdit->showAlertMessage(tr("Please enter a command"), m_commandLineEdit->parentWidget());
        return;
    }

    m_newAction = new QAction(ShortcutManager::instance());
    m_newAction->setObjectName("CustomQAction");//Add by ut001000 renfeixiang 2020-08-13
    m_newAction->setText(strName);
    m_newAction->setData(strCommand);
    m_newAction->setShortcut(m_shortCutLineEdit->keySequence());

    /************************ Mod by m000743 sunchengxi 2020-04-21:自定义命令修改的异常问题 Begin************************/
    QAction *existAction = nullptr;
    int icount = 0;
    if (CCT_MODIFY == m_type) {
        qCInfo(customcommand) << "It is the modify type of custom command operation";
        if (m_bRefreshCheck) {
            qCInfo(customcommand) << "CustomCommand was refreshed";
            QAction *refreshExitAction = nullptr;
            refreshExitAction = ShortcutManager::instance()->checkActionIsExist(*m_newAction);
            if (refreshExitAction) {
                qCWarning(customcommand) << "The custom command already exists";
                accept();
                return;
            }
        }

        if (strName == m_currItemData->m_cmdName && strCommand == m_currItemData->m_cmdText && keytmp == QKeySequence(m_currItemData->m_cmdShortcut)) {
            qCInfo(customcommand) << "The custom command don't need to save again.";
            accept();
            return;
        }

        existAction = ShortcutManager::instance()->checkActionIsExistForModify(*m_newAction);

        if (strName != m_currItemData->m_cmdName) {
            qCInfo(customcommand) << "Custom commands have been changed.";
            QList<QAction *> &customCommandActionList = ShortcutManager::instance()->getCustomCommandActionList();
            for (int i = 0; i < customCommandActionList.size(); i++) {
                // qCDebug(customcommand) << "Loop iteration:" << i;
                QAction *curAction = customCommandActionList[i];
                QString strCmdName = curAction->text();
                if (strCmdName == strName) {
                    // qCDebug(customcommand) << "found matching command name";
                    icount++;
                }
            }
        }
    } else {
        qCInfo(customcommand) << "It is the add type of custom command operation";
        existAction = ShortcutManager::instance()->checkActionIsExist(*m_newAction);
    }

    if (nullptr != existAction || icount) {
        qCInfo(customcommand) << "The name already exists";
        QString strFistLine = tr("The name already exists,");
        QString strSecondeLine = tr("please input another one.");
        Utils::showSameNameDialog(this, strFistLine, strSecondeLine);
    } else {
        qCDebug(customcommand) << "accepting dialog";
        accept();
    }
    /************************ Mod by m000743 sunchengxi 2020-04-21:自定义命令修改的异常问题 End  ************************/
}

bool CustomCommandOptDlg::checkSequence(const QKeySequence &sequence)
{
    qCDebug(customcommand) << "Enter checkSequence";
    QString checkName = m_nameLineEdit->text();

    if (sequence.toString().isEmpty()) {
        qCDebug(customcommand) << "sequence is empty";
        return true;
    }

    QString reason;
    // 判断快捷键是否冲突
    if (!ShortcutManager::instance()->checkShortcutValid(checkName, sequence.toString(), reason)) {
        qCDebug(customcommand) << "shortcut key conflict in checkSequence";
        // 冲突
        if (sequence.toString() != "Esc") {
            qCDebug(customcommand) << "sequence is not Esc in checkSequence";
            showShortcutConflictMsgbox(reason);
        }

        m_shortCutLineEdit->clear();
        m_shortCutLineEdit->setKeySequence(QKeySequence(m_lastCmdShortcut));

        QTimer::singleShot(30, this, [&]() {
            m_shortCutLineEdit->setFocus();
        });
        return false;
    }

    return true;
}

void CustomCommandOptDlg::setModelIndex(const QModelIndex &mi)
{
    // qCDebug(customcommand) << "Enter setModelIndex";
    modelIndex = mi;
}

void CustomCommandOptDlg::slotDelCurCustomCommand()
{
    // qCDebug(customcommand) << "Enter slotDelCurCustomCommand";
    qCInfo(customcommand) << "Deleting current custom command";
    m_bDelOpt = true;
    reject();
}

void CustomCommandOptDlg::addCancelConfirmButtons()
{
    // qCDebug(customcommand) << "Enter addCancelConfirmButtons";
    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->setSpacing(9);
    buttonsLayout->setContentsMargins(10, 0, 10, 0);

    QFont btnFont;
    m_cancelBtn = new DPushButton(this);
    m_cancelBtn->setObjectName("CustomCancelButton");//Add by ut001000 renfeixiang 2020-08-13
    m_cancelBtn->setFixedWidth(209);
    m_cancelBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_cancelBtn->setFont(btnFont);

    m_confirmBtn = new DSuggestButton(this);
    m_confirmBtn->setObjectName("CustomConfirmButton");//Add by ut001000 renfeixiang 2020-08-13
    m_confirmBtn->setFixedWidth(209);
    m_confirmBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_confirmBtn->setFont(btnFont);

    //设置回车键默认响应的按钮
    m_confirmBtn->setDefault(true);

    setTabOrder(m_confirmBtn, m_closeButton);//设置右上角关闭按钮的tab键控制顺序

    m_verticalLine = new DVerticalLine(this);
    DPalette pa = DPaletteHelper::instance()->palette(m_verticalLine);
    QColor splitColor = pa.color(DPalette::ItemBackground);
    pa.setBrush(DPalette::Window, splitColor);
    m_verticalLine->setPalette(pa);
    m_verticalLine->setBackgroundRole(QPalette::Window);
    m_verticalLine->setAutoFillBackground(true);
    m_verticalLine->setFixedSize(3, 28);

    buttonsLayout->addWidget(m_cancelBtn);
    buttonsLayout->addWidget(m_verticalLine);
    buttonsLayout->addWidget(m_confirmBtn);
    /************************ Add by m000743 sunchengxi 2020-04-15:默认enter回车按下，走确认校验流程 Begin************************/
    m_confirmBtn->setDefault(true);
    /************************ Add by m000743 sunchengxi 2020-04-15:默认enter回车按下，走确认校验流程 End ************************/

    connect(m_cancelBtn, &DPushButton::clicked, this, &CustomCommandOptDlg::slotCancelBtnClicked);

    connect(m_confirmBtn, &DPushButton::clicked, this, &CustomCommandOptDlg::slotConfirmBtnClicked);

    /************************ Add by m000743 sunchengxi 2020-04-21:快捷键编辑结束后，enter按下可以响应校验 Begin************************/
    connect(m_shortCutLineEdit, &DKeySequenceEdit::editingFinished, this, &CustomCommandOptDlg::slotShortCutLineEditFinished);
    /************************ Add by m000743 sunchengxi 2020-04-21:快捷键编辑结束后，enter按下可以响应校验 End  ************************/

    m_mainLayout->addLayout(buttonsLayout);
}

inline void CustomCommandOptDlg::slotCancelBtnClicked()
{
    // qCDebug(customcommand) << "Enter slotCancelBtnClicked";
    m_confirmResultCode = QDialog::Rejected;
    reject();
}

inline void CustomCommandOptDlg::slotConfirmBtnClicked()
{
    // qCDebug(customcommand) << "Enter slotConfirmBtnClicked";
    m_confirmResultCode = QDialog::Accepted;
    emit confirmBtnClicked();
}

inline void CustomCommandOptDlg::slotShortCutLineEditFinished()
{
    // qCDebug(customcommand) << "Enter slotShortCutLineEditFinished";
    /******** Add by nt001000 renfeixiang 2020-05-14:修改快捷框输入后，添加（m_confirmBtn）按钮高亮问题,将光标从添加按钮取消，设置到快捷框上 Begin***************/
    m_shortCutLineEdit->setFocus();
    /******** Add by nt001000 renfeixiang 2020-05-14:修改快捷框输入后，添加（m_confirmBtn）按钮高亮问题,将光标从添加按钮取消，设置到快捷框上 End***************/
}

QVBoxLayout *CustomCommandOptDlg::getMainLayout()
{
    // qCDebug(customcommand) << "Enter getMainLayout";
    return m_mainLayout;
}

QDialog::DialogCode CustomCommandOptDlg::getConfirmResult()
{
    // qCDebug(customcommand) << "Enter getConfirmResult";
    return m_confirmResultCode;
}

/*******************************************************************************
 1. @函数:    setLogoVisable
 2. @作者:    sunchengxi
 3. @日期:    2020-07-31
 4. @说明:    设置自定义命令操作窗口的图标是否可见
*******************************************************************************/
//void CustomCommandOptDlg::setLogoVisable(bool visible)
//{
//    if (nullptr != m_logoIcon) {
//        m_logoIcon->setVisible(visible);
//    }
//}

void CustomCommandOptDlg::setTitle(const QString &title)
{
    // qCDebug(customcommand) << "Enter setTitle";
    if (nullptr != m_titleText) {
        qCDebug(customcommand) << "m_titleText is not null";
        m_titleText->setText(title);
    }
}

QLayout *CustomCommandOptDlg::getContentLayout()
{
    // qCDebug(customcommand) << "Enter getContentLayout";
    return m_contentLayout;
}

void CustomCommandOptDlg::setCancelBtnText(const QString &strCancel)
{
    // qCDebug(customcommand) << "Enter setCancelBtnText";
    m_cancelBtn->setText(strCancel);
    Utils::setSpaceInWord(m_cancelBtn);
}

void CustomCommandOptDlg::setConfirmBtnText(const QString &strConfirm)
{
    // qCDebug(customcommand) << "Enter setConfirmBtnText";
    m_confirmBtn->setText(strConfirm);
    Utils::setSpaceInWord(m_confirmBtn);
}

void CustomCommandOptDlg::showShortcutConflictMsgbox(QString txt)
{
    qCDebug(customcommand) << "Enter showShortcutConflictMsgbox";
    /******** Modify by ut000610 daizhengwen 2020-05-27: 出现提示和快捷键显示不一致的问题 bug#28507****************/
    // fix#bug 37399
    for (QString key : ShortcutManager::instance()->m_mapReplaceText.keys()) {
        // qCDebug(customcommand) << "Loop iteration for key:" << key;
        if (txt.contains(key)) {
            // qCDebug(customcommand) << "txt contains key";
            txt.replace(key, ShortcutManager::instance()->m_mapReplaceText[key]);
        }
    }
    /********************* Modify by ut000610 daizhengwen End ************************/
    // 若没有弹窗，初始化
    if (nullptr == m_shortcutConflictDialog) {
        qCDebug(customcommand) << "m_shortcutConflictDialog is null, initializing";
        m_shortcutConflictDialog = new DDialog(this);
        m_shortcutConflictDialog->setObjectName("CustomShortcutConflictDialog");//Add by ut001000 renfeixiang 2020-08-13
        /******** Modify by nt001000 renfeixiang 2020-05-29:修改 因为弹框改为非模态之后，快捷框冲突选中快捷框功能移动这 Begin***************/
        connect(m_shortcutConflictDialog, &DDialog::finished, this, &CustomCommandOptDlg::slotShortcutConflictDialogFinished);
        /******** Modify by nt001000 renfeixiang 2020-05-29:修改 修改因为弹框改为非模态之后，快捷框冲突选中快捷框功能移动这 End***************/
        m_shortcutConflictDialog->setIcon(QIcon::fromTheme("dialog-warning"));
        /***mod by ut001121 zhangmeng 20200521 将确认按钮设置为默认按钮 修复BUG26960***/
        m_shortcutConflictDialog->addButton(QString(tr("OK", "button")), true, DDialog::ButtonNormal);
    }
    // 存在后显示
    m_shortcutConflictDialog->setTitle(QString(txt + QObject::tr("please set another one.")));
    m_shortcutConflictDialog->show();
}

inline void CustomCommandOptDlg::slotShortcutConflictDialogFinished()
{
    // qCDebug(customcommand) << "Enter slotShortcutConflictDialogFinished";
    m_shortcutConflictDialog->hide();
    /******** Add by nt001000 renfeixiang 2020-05-14:快捷框输入已经存在的快捷后，快捷框依然是选中状态 Begin***************/
    QTimer::singleShot(30, this, &CustomCommandOptDlg::slotSetShortCutLineEditFocus);
    /******** Add by nt001000 renfeixiang 2020-05-14:快捷框输入已经存在的快捷后，快捷框依然是选中状态 End***************/
}

inline void CustomCommandOptDlg::slotSetShortCutLineEditFocus()
{
    // qCDebug(customcommand) << "Enter slotSetShortCutLineEditFocus";
    m_shortCutLineEdit->setFocus();
}

/**
 * @brief 接收 DGuiApplicationHelper::sizeModeChanged() 信号, 根据不同的布局模式调整
 *      当前界面的布局. 只能在界面创建完成后调用.
 */
void CustomCommandOptDlg::updateSizeMode()
{
    // qCDebug(customcommand) << "Enter updateSizeMode";
#ifdef DTKWIDGET_CLASS_DSizeMode
    if (DGuiApplicationHelper::isCompactMode()) {
        qCDebug(customcommand) << "compact mode";
        m_titleBar->setFixedHeight(WIN_TITLE_BAR_HEIGHT_COMPACT);
        m_logoIcon->setFixedSize(QSize(ICONSIZE_40_COMPACT, ICONSIZE_40_COMPACT));
        m_closeButton->setIconSize(QSize(ICONSIZE_40_COMPACT, ICONSIZE_40_COMPACT));
        m_verticalLine->setFixedSize(VERTICAL_WIDTH_COMPACT, VERTICAL_HEIGHT_COMPACT);
        if (m_deleteCmdWidget) {
            qCDebug(customcommand) << "m_deleteCmdWidget is not null in compact mode";
            m_deleteCmdWidget->setFixedHeight(m_iFixedHeightCompact);
        }

    } else {
        qCDebug(customcommand) << "normal mode";
        m_titleBar->setFixedHeight(WIN_TITLE_BAR_HEIGHT);
        m_logoIcon->setFixedSize(QSize(ICONSIZE_50, ICONSIZE_50));
        m_closeButton->setIconSize(QSize(ICONSIZE_50, ICONSIZE_50));
        m_verticalLine->setFixedSize(VERTICAL_WIDTH, VERTICAL_HEIGHT);
        if (m_deleteCmdWidget) {
            qCDebug(customcommand) << "m_deleteCmdWidget is not null in normal mode";
            m_deleteCmdWidget->setFixedHeight(m_iFixedHeight);
        }
    }

    if (layout()) {
        qCDebug(customcommand) << "layout is not null";
        layout()->invalidate();
    }

    updateGeometry();
    // 根据新界面布局，刷新界面大小
    QTimer::singleShot(0, this, [=](){ resize(SETTING_DIALOG_WIDTH, minimumSizeHint().height()); });
#endif
}

void CustomCommandOptDlg::addContent(QWidget *content)
{
    // qCDebug(customcommand) << "Enter addContent";
    Q_ASSERT(nullptr != m_contentLayout);

    m_contentLayout->addWidget(content);
}

void CustomCommandOptDlg::setIconPixmap(const QPixmap &iconPixmap)
{
    // qCDebug(customcommand) << "Enter setIconPixmap";
    if (nullptr != m_logoIcon) {
        qCDebug(customcommand) << "m_logoIcon is not null";
        m_logoIcon->setVisible(true);
        m_logoIcon->setPixmap(iconPixmap);
    }
}

void CustomCommandOptDlg::closeEvent(QCloseEvent *event)
{
    // qCDebug(customcommand) << "Enter closeEvent";
    Q_UNUSED(event)

    done(-1);

    Q_EMIT closed();
}

void CustomCommandOptDlg::slotRefreshData(QString oldCmdName, QString newCmdName)
{
    // qCDebug(customcommand) << "Enter slotRefreshData";
    if (CCT_ADD == m_type) {
        qCWarning(customcommand) << "Currently is the add operation interface";
        return;
    }
    //不进行刷新操作
    if (oldCmdName.isEmpty() && newCmdName.isEmpty()) {
        qCDebug(customcommand) << "both oldCmdName and newCmdName are empty";
        return;
    }

    //当前的自定义命令的名称 不是被修改的自定义名称时，不进行刷新操作
    if (m_currItemData->m_cmdName != oldCmdName) {
        qCDebug(customcommand) << "current command name doesn't match oldCmdName";
        return;
    }
    m_bRefreshCheck = true;
    qCInfo(customcommand) << "Refresh custom command data.Curren command name is " <<  m_nameLineEdit->text();

    QAction *currAction = new QAction(ShortcutManager::instance());
    currAction->setText(newCmdName);
    QAction *existAction = nullptr;
    existAction = ShortcutManager::instance()->checkActionIsExist(*currAction);
    //根据 newCmdName 查找自定义命令
    if (nullptr == existAction) {
        qCDebug(customcommand) << "existAction is null";
        //不存在，则认为当前自定义命令在其他编辑窗口中删除了，如果打开了删除确认界面，则关闭删除确认界面。关闭当前编辑窗口
        delete currAction;
        if (m_dlgDelete && m_dlgDelete->isVisible()) {
            qCDebug(customcommand) << "m_dlgDelete is visible, closing";
            m_dlgDelete->close();
            m_dlgDelete = nullptr;
        }

        close();
    } else {
        qCDebug(customcommand) << "existAction is not null, updating data";
        //当前自定义命令在其他窗口修改，则更新数据
        delete currAction;
        m_nameLineEdit->setText(existAction->text());
        m_commandLineEdit->setText(existAction->data().toString());
        m_shortCutLineEdit->setKeySequence(existAction->shortcut());
        m_currItemData->m_cmdName = newCmdName;
    }

}

void CustomCommandOptDlg::closeRefreshDataConnection()
{
    // qCDebug(customcommand) << "Enter closeRefreshDataConnection";
    disconnect(Service::instance(), &Service::refreshCommandPanel, this, &CustomCommandOptDlg::slotRefreshData);
}
