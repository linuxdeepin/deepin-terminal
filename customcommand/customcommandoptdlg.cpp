#include "customcommandoptdlg.h"
#include "termcommandlinkbutton.h"

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

#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

CustomCommandOptDlg::CustomCommandOptDlg(CustomCmdOptType type, QAction *curAction, QWidget *parent)
    : TermBaseDialog(parent),
      m_type(type),
      m_action(curAction),
      m_nameLineEdit(new DLineEdit),
      m_commandLineEdit(new DLineEdit),
      m_shortCutLineEdit(new DKeySequenceEdit),
      m_bDelOpt(false)
{
    int contentOffsetY = 18;
    QWidget *contentFrame = new QWidget;
    contentFrame->setFixedSize(459, 128 + contentOffsetY);

    QVBoxLayout *contentLayout = new QVBoxLayout;
    contentLayout->setSpacing(0);
    contentLayout->setContentsMargins(0, 0, 0, 0);

    QWidget *nameFrame = new QWidget;
    QHBoxLayout *nameLayout = new QHBoxLayout;
    nameLayout->setSpacing(0);
    nameLayout->setContentsMargins(0, 0, 0, 0);
    nameFrame->setLayout(nameLayout);

    QWidget *cmdFrame = new QWidget;
    QHBoxLayout *cmdLayout = new QHBoxLayout;
    cmdLayout->setSpacing(0);
    cmdLayout->setContentsMargins(0, 0, 0, 0);
    cmdFrame->setLayout(cmdLayout);

    QWidget *scFrame = new QWidget;
    QHBoxLayout *scLayout = new QHBoxLayout;
    scLayout->setSpacing(0);
    scLayout->setContentsMargins(0, 0, 0, 0);
    scFrame->setLayout(scLayout);

    DLabel *nameLabel = new DLabel(tr("Name:"));
    nameLabel->setFixedWidth(100);

    DLabel *cmdLabel = new DLabel(tr("Command:"));
    cmdLabel->setFixedWidth(100);

    DLabel *shortCutLabel = new DLabel(tr("Shortcuts:"));
    shortCutLabel->setFixedWidth(100);

    m_nameLineEdit->setFixedSize(285, 36);
    m_commandLineEdit->setFixedSize(285, 36);
    m_shortCutLineEdit->setFixedSize(285, 36);

    m_nameLineEdit->lineEdit()->setPlaceholderText(tr("Mandatory"));
    m_commandLineEdit->lineEdit()->setPlaceholderText(tr("Mandatory"));
    m_shortCutLineEdit->ShortcutDirection(Qt::AlignLeft);

    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(m_nameLineEdit);

    cmdLayout->addWidget(cmdLabel);
    cmdLayout->addWidget(m_commandLineEdit);

    scLayout->addWidget(shortCutLabel);
    scLayout->addWidget(m_shortCutLineEdit);

    contentLayout->addSpacing(contentOffsetY);
    contentLayout->addWidget(nameFrame);
    contentLayout->addWidget(cmdFrame);
    contentLayout->addWidget(scFrame);

    contentFrame->setLayout(contentLayout);

    connect(m_nameLineEdit, &DLineEdit::editingFinished, this, [ = ] {
        if (m_nameLineEdit->text().isEmpty())
        {
            m_nameLineEdit->lineEdit()->setPlaceholderText(tr("Mandatory"));
        }
    });

    connect(m_commandLineEdit, &DLineEdit::editingFinished, this, [ = ] {
        if (m_commandLineEdit->text().isEmpty())
        {
            m_commandLineEdit->lineEdit()->setPlaceholderText(tr("Mandatory"));
        }
    });

    addContent(contentFrame);

    if (m_type == CCT_ADD) {
        setFixedSize(459, 262);
        setTitle(tr("Add Command"));

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

        QString strName = m_action->text();
        QString strCommad = m_action->data().toString();
        QKeySequence keyseq = m_action->shortcut();
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
        setConfirmBtnText(tr("Save1"));
    }

    connect(this, &TermBaseDialog::confirmBtnClicked, this, &CustomCommandOptDlg::slotAddSaveButtonClicked);

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

CustomCommandOptDlg::~CustomCommandOptDlg()
{
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

    if (strName.isEmpty() || strCommand.isEmpty()) {
        return;
    }

    m_newAction = new QAction(this);
    m_newAction->setText(strName);
    m_newAction->setData(strCommand);
    m_newAction->setShortcut(m_shortCutLineEdit->keySequence());
    accept();
}

void CustomCommandOptDlg::slotDelCurCustomCommand()
{
    m_bDelOpt = true;
    reject();
}
