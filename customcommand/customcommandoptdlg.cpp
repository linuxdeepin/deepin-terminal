#include "customcommandoptdlg.h"
#include "termcommandlinkbutton.h"
#include "shortcutmanager.h"
#include "shortcutmanager.h"
#include "utils.h"

#include <DButtonBox>
#include <DPushButton>
#include <DGroupBox>
#include <DLabel>
#include <DKeySequenceEdit>
#include <DCommandLinkButton>
#include <DDialogCloseButton>
#include <DApplicationHelper>
#include <DWidgetUtil>
#include <DLog>
#include <DFontSizeManager>
#include <DVerticalLine>
#include <DGuiApplicationHelper>

#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QApplication>
#include <QClipboard>

CustomCommandOptDlg::CustomCommandOptDlg(CustomCmdOptType type, CustomCommandItemData *currItemData, QWidget *parent)
    : DAbstractDialog(parent),
      m_type(type),
      m_currItemData(currItemData),
      m_nameLineEdit(new DLineEdit),
      m_commandLineEdit(new DLineEdit),
      m_shortCutLineEdit(new DKeySequenceEdit),
      m_bDelOpt(false)
{
    initUITitle();
    initTitleConnections();
    initUI();
}

CustomCommandOptDlg::~CustomCommandOptDlg()
{
}

void CustomCommandOptDlg::initUI()
{
    // 设置弹窗模态
    setWindowModality(Qt::WindowModal);

    QWidget *contentFrame = new QWidget;

    QVBoxLayout *contentLayout = new QVBoxLayout;
    contentLayout->setSpacing(10);
    contentLayout->setContentsMargins(0, 0, 0, 0);

    QWidget *nameFrame = new QWidget;
    nameFrame->setFixedWidth(459);
    QHBoxLayout *nameLayout = new QHBoxLayout;
    nameLayout->setSpacing(0);
    nameLayout->setContentsMargins(28, 0, 30, 0);
    nameFrame->setLayout(nameLayout);

    QWidget *cmdFrame = new QWidget;
    QHBoxLayout *cmdLayout = new QHBoxLayout;
    cmdLayout->setSpacing(0);
    cmdLayout->setContentsMargins(28, 0, 30, 0);
    cmdFrame->setLayout(cmdLayout);

    QWidget *scFrame = new QWidget;
    QHBoxLayout *scLayout = new QHBoxLayout;
    scLayout->setSpacing(0);
    scLayout->setContentsMargins(28, 0, 30, 0);
    scFrame->setLayout(scLayout);

    DLabel *nameLabel = new DLabel(tr("Name:"));

    DLabel *cmdLabel = new DLabel(tr("Command:"));

    DLabel *shortCutLabel = new DLabel(tr("Shortcuts:"));

    m_nameLineEdit->setFixedWidth(285);
    m_commandLineEdit->setFixedWidth(285);
    m_shortCutLineEdit->setFixedWidth(285);

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

    connect(m_nameLineEdit, &DLineEdit::editingFinished, this, [ = ] {
        if (m_nameLineEdit->text().isEmpty())
        {
            m_nameLineEdit->lineEdit()->setPlaceholderText(tr("Required"));
        }
    });

    connect(m_commandLineEdit, &DLineEdit::editingFinished, this, [ = ] {
        if (m_commandLineEdit->text().isEmpty())
        {
            m_commandLineEdit->lineEdit()->setPlaceholderText(tr("Required"));
        }
    });

    addContent(contentFrame);
    if (m_type == CCT_ADD) {
        setFixedSize(459, 262);
        setTitle(tr("Add Command"));
        initCommandFromClipBoardText();

        getMainLayout()->addSpacing(18);
    } else {
        setFixedSize(459, 296);
        setTitle(tr("Edit Command"));

        QWidget *deleteCmdWidget = new QWidget;
        deleteCmdWidget->setFixedHeight(54);

        QHBoxLayout *deleteCmdLayout = new QHBoxLayout();
        deleteCmdLayout->setSpacing(0);
        deleteCmdLayout->setContentsMargins(0, 0, 0, 0);
        TermCommandLinkButton *deleteCmdBtn = new TermCommandLinkButton();
        deleteCmdBtn->setText(tr("Delete Command"));

        deleteCmdLayout->addStretch();
        deleteCmdLayout->addWidget(deleteCmdBtn);
        deleteCmdLayout->addStretch();
        deleteCmdWidget->setLayout(deleteCmdLayout);

        getMainLayout()->addWidget(deleteCmdWidget);

        connect(deleteCmdBtn, &DCommandLinkButton::clicked, this, &CustomCommandOptDlg::slotDelCurCustomCommand);

        QString strName = m_currItemData->m_cmdName;
        QString strCommad = m_currItemData->m_cmdText;
        QKeySequence keyseq = QKeySequence(m_currItemData->m_cmdShortcut);
        m_lastCmdShortcut = m_currItemData->m_cmdShortcut;
        m_nameLineEdit->setText(strName);
        m_commandLineEdit->setText(strCommad);
        m_shortCutLineEdit->setKeySequence(keyseq);

#ifdef UI_DEBUG
        deleteCmdWidget->setStyleSheet("background:green");
#endif
    }

    addCancelConfirmButtons();
    setCancelBtnText(tr("Cancel"));
    if (m_type == CCT_ADD) {
        setConfirmBtnText(tr("Add"));
    } else {
        setConfirmBtnText(tr("Save"));
    }
    m_lastCmdShortcut = m_shortCutLineEdit->keySequence().toString();
    connect(this, &CustomCommandOptDlg::confirmBtnClicked, this, &CustomCommandOptDlg::slotAddSaveButtonClicked);
    connect(m_shortCutLineEdit, &KeySequenceEdit::editingFinished, [ = ](const QKeySequence & sequence) {
        //删除
        if (sequence.toString() == "Backspace") {
            m_shortCutLineEdit->clear();
            m_lastCmdShortcut = "";
            return ;
        }
        // 取消
        if (sequence.toString() == "Esc") {
            m_shortCutLineEdit->clear();
            m_shortCutLineEdit->setKeySequence(QKeySequence(m_lastCmdShortcut));
            return ;
        }
        QString checkName;
        //QString seq = m_shortCutLineEdit->text();
        if (m_type != CCT_ADD) {
            checkName = m_nameLineEdit->text();
        }
        if (!ShortcutManager::instance()->isValidShortcut(checkName, sequence.toString())) {
            m_shortCutLineEdit->clear();
            m_shortCutLineEdit->setKeySequence(QKeySequence(m_lastCmdShortcut));
            return;
        }
        m_lastCmdShortcut = sequence.toString();
    });

#ifdef UI_DEBUG
    contentFrame->setStyleSheet("background:cyan");
    nameLabel->setStyleSheet("background:#0000ff");
    cmdLabel->setStyleSheet("background:#00ff00");
    shortCutLabel->setStyleSheet("background:yellow");

    m_nameLineEdit->setStyleSheet("background:#ff00ff");
    m_commandLineEdit->setStyleSheet("background:#ff00ff");
    m_shortCutLineEdit->setStyleSheet("background:#ff00ff");
#endif
}

// fix bug 18366 终端中选中内容，创建自定义命令，选中内容没有自动粘贴到命令输入框
void CustomCommandOptDlg::initCommandFromClipBoardText()
{
    if (m_commandLineEdit) {
        QString clipText = ShortcutManager::instance()->getClipboardCommandData();
        m_commandLineEdit->setText(clipText.trimmed());
    }
}

QAction *CustomCommandOptDlg::getCurCustomCmd()
{
    return m_newAction;
}

bool CustomCommandOptDlg::isDelCurCommand()
{
    return m_bDelOpt;
}

void CustomCommandOptDlg::slotAddSaveButtonClicked()
{
    QString strName = m_nameLineEdit->text();
    QString strCommand = m_commandLineEdit->text();
    QKeySequence keytmp = m_shortCutLineEdit->keySequence();

    if (strName.isEmpty()) {
        m_nameLineEdit->showAlertMessage(tr("Please enter name"), m_nameLineEdit->parentWidget());
        return;
    }

    if (strCommand.isEmpty()) {
        m_commandLineEdit->showAlertMessage(tr("Please enter command"), m_commandLineEdit->parentWidget());
        return;
    }

    m_newAction = new QAction(this);
    m_newAction->setText(strName);
    m_newAction->setData(strCommand);
    m_newAction->setShortcut(m_shortCutLineEdit->keySequence());

    /************************ Mod by m000743 sunchengxi 2020-04-21:自定义命令修改的异常问题 Begin************************/
    QAction *existAction = nullptr;
    int icount = 0;
    m_bNeedDel = false;
    if (m_type == CCT_MODIFY) {

        if (strName == m_currItemData->m_cmdName && strCommand == m_currItemData->m_cmdText && keytmp == QKeySequence(m_currItemData->m_cmdShortcut)) {
            accept();
            return;
        }

        existAction = ShortcutManager::instance()->checkActionIsExistForModify(*m_newAction);

        if (strName != m_currItemData->m_cmdName) {
            m_bNeedDel = true;
            QList<QAction *> &customCommandActionList = ShortcutManager::instance()->getCustomCommandActionList();
            for (int i = 0; i < customCommandActionList.size(); i++) {
                QAction *curAction = customCommandActionList[i];
                QString strCmdName = curAction->text();
                if (strCmdName == strName) {
                    icount++;
                }
            }
        }
    } else {
        existAction = ShortcutManager::instance()->checkActionIsExist(*m_newAction);
    }

    if (nullptr != existAction || icount) {
        QString strFistLine = tr("The name already exists,");
        QString strSecondeLine = tr("please input another one.");
        Utils::showSameNameDialog(strFistLine, strSecondeLine);
    } else {
        accept();
    }
    /************************ Mod by m000743 sunchengxi 2020-04-21:自定义命令修改的异常问题 End  ************************/
}

void CustomCommandOptDlg::slotDelCurCustomCommand()
{
    m_bDelOpt = true;
    reject();
}

void CustomCommandOptDlg::initUITitle()
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 10);

    QHBoxLayout *titleLayout = new QHBoxLayout();
    titleLayout->setSpacing(0);
    titleLayout->setContentsMargins(0, 0, 0, 0);

    m_titleBar = new QWidget(this);
    m_titleBar->setFixedHeight(50);
    m_titleBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_titleBar->setLayout(titleLayout);

    m_logoIcon = new DLabel(this);
    m_logoIcon->setFixedSize(QSize(50, 50));
    m_logoIcon->setFocusPolicy(Qt::NoFocus);
    m_logoIcon->setAttribute(Qt::WA_TransparentForMouseEvents);

    m_closeButton = new DWindowCloseButton(this);
    m_closeButton->setFocusPolicy(Qt::NoFocus);
    m_closeButton->setIconSize(QSize(50, 50));

    m_titleText = new DLabel(this);
    m_titleText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_titleText->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(m_titleText, DFontSizeManager::T5, QFont::DemiBold);
    // 字色
    DPalette palette = m_titleText->palette();
//    palette.setColor(QPalette::WindowText, palette.color(DPalette::TextTitle));
    QColor color;
    if (DApplicationHelper::instance()->themeType() == DApplicationHelper::DarkType) {
        color = QColor::fromRgb(192, 198, 212, 255);
    } else {
        color = QColor::fromRgb(0, 26, 46, 255);
    }
    palette.setBrush(QPalette::WindowText, color);
    m_titleText->setPalette(palette);

    titleLayout->addWidget(m_logoIcon, 0, Qt::AlignLeft | Qt::AlignVCenter);
    titleLayout->addWidget(m_titleText, 0, Qt::AlignHCenter);
    titleLayout->addWidget(m_closeButton, 0, Qt::AlignRight | Qt::AlignTop);

    //Dialog content
    m_contentLayout = new QVBoxLayout();
    m_contentLayout->setSpacing(0);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);

    m_content = new QWidget(this);
    m_content->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_content->setLayout(m_contentLayout);

    mainLayout->addWidget(m_titleBar, 0, Qt::AlignTop);
    mainLayout->addWidget(m_content);
    setLayout(mainLayout);

    m_mainLayout = mainLayout;
}

void CustomCommandOptDlg::addCancelConfirmButtons()
{
    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->setSpacing(9);
    buttonsLayout->setContentsMargins(10, 0, 10, 0);

    QFont btnFont;
    m_cancelBtn = new DPushButton(this);
    m_cancelBtn->setFixedWidth(209);
    m_cancelBtn->setFixedHeight(36);
    m_cancelBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_cancelBtn->setFont(btnFont);

    m_confirmBtn = new DSuggestButton(this);
    m_confirmBtn->setFixedWidth(209);
    m_confirmBtn->setFixedHeight(36);
    m_confirmBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_confirmBtn->setFont(btnFont);

    //设置回车键默认响应的按钮
    m_confirmBtn->setDefault(true);

    DVerticalLine *verticalLine = new DVerticalLine(this);
    DPalette pa = DApplicationHelper::instance()->palette(verticalLine);
    QColor splitColor = pa.color(DPalette::ItemBackground);
    pa.setBrush(DPalette::Background, splitColor);
    verticalLine->setPalette(pa);
    verticalLine->setBackgroundRole(QPalette::Background);
    verticalLine->setAutoFillBackground(true);
    verticalLine->setFixedSize(3, 28);

    buttonsLayout->addWidget(m_cancelBtn);
    buttonsLayout->addWidget(verticalLine);
    buttonsLayout->addWidget(m_confirmBtn);
    /************************ Add by m000743 sunchengxi 2020-04-15:默认enter回车按下，走确认校验流程 Begin************************/
    m_confirmBtn->setDefault(true);
    /************************ Add by m000743 sunchengxi 2020-04-15:默认enter回车按下，走确认校验流程 End ************************/

    connect(m_cancelBtn, &DPushButton::clicked, this, [ = ]() {
        qDebug() << "cancelBtnClicked";
        m_confirmResultCode = QDialog::Rejected;
        reject();
        close();
    });

    connect(m_confirmBtn, &DPushButton::clicked, this, [ = ]() {
        qDebug() << "confirmBtnClicked";
        m_confirmResultCode = QDialog::Accepted;
        emit confirmBtnClicked();
    });

    /************************ Add by m000743 sunchengxi 2020-04-21:快捷键编辑结束后，enter按下可以响应校验 Begin************************/
    connect(m_shortCutLineEdit, &DKeySequenceEdit::editingFinished, this, [ = ]() {
        qDebug() << "shourtCut editingFinished";
        m_confirmBtn->setFocus();
    });
    /************************ Add by m000743 sunchengxi 2020-04-21:快捷键编辑结束后，enter按下可以响应校验 End  ************************/

    m_mainLayout->addLayout(buttonsLayout);
}

QVBoxLayout *CustomCommandOptDlg::getMainLayout()
{
    return m_mainLayout;
}

void CustomCommandOptDlg::initTitleConnections()
{
    connect(m_closeButton, &DWindowCloseButton::clicked, this, [this]() {
        this->close();
    });
}

QDialog::DialogCode CustomCommandOptDlg::getConfirmResult()
{
    return m_confirmResultCode;
}

void CustomCommandOptDlg::setLogoVisable(bool visible)
{
    if (nullptr != m_logoIcon) {
        m_logoIcon->setVisible(visible);
    }
}

void CustomCommandOptDlg::setTitle(const QString &title)
{
    if (nullptr != m_titleText) {
        m_titleText->setText(title);
    }
}

QLayout *CustomCommandOptDlg::getContentLayout()
{
    return m_contentLayout;
}

void CustomCommandOptDlg::setCancelBtnText(const QString &strCancel)
{
    m_cancelBtn->setText(strCancel);
    Utils::setSpaceInWord(m_cancelBtn);
}

void CustomCommandOptDlg::setConfirmBtnText(const QString &strConfirm)
{
    m_confirmBtn->setText(strConfirm);
    Utils::setSpaceInWord(m_confirmBtn);
}

void CustomCommandOptDlg::addContent(QWidget *content)
{
    Q_ASSERT(nullptr != getContentLayout());

    getContentLayout()->addWidget(content);
}

void CustomCommandOptDlg::setIconPixmap(const QPixmap &iconPixmap)
{
    if (nullptr != m_logoIcon) {
        m_logoIcon->setVisible(true);
        m_logoIcon->setPixmap(iconPixmap);
    }
}

void CustomCommandOptDlg::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)

    done(-1);

    Q_EMIT closed();
}
