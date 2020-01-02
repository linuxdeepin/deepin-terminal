#include "customcommandoptdlg.h"
#include <DButtonBox>
#include <DPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <DGroupBox>
#include <DLabel>
#include <QGridLayout>
#include <DKeySequenceEdit>
#include <DCommandLinkButton>
#include <QDebug>
#include <DDialogCloseButton>
#include <DApplicationHelper>
#include <DWidgetUtil>
//#include <Dialog>
CustomCommandOptDlg::CustomCommandOptDlg(CustomCmdOptType type, QAction *curAction, QWidget *parent) : DAbstractDialog(parent)
    , m_type(type)
    , m_action(curAction)
    , m_nameLineEdit(new DLineEdit)
    , m_commandLineEdit(new DLineEdit)
    , m_shortCutsLineEdit(new DKeySequenceEdit)
    , m_bDelOpt(false)
{
    //setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint | Qt::WindowMinMaxButtonsHint|Qt::WindowTitleHint);
    //setWindowFlags(windowFlags() &~Qt::WindowMinMaxButtonsHint&~Qt::WindowContextHelpButtonHint &~Qt::MaximizeUsingFullscreenGeometryHint&~Qt::WindowMaximizeButtonHint);
    //etWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
    // setWindowFlags(Qt::FramelessWindowHint);
    //Qt::WindowFlags flags=Qt::Dialog;
    //setWindowFlags(flags);

    setFixedSize(300, 200);
    QHBoxLayout *pHBoxLayout = new QHBoxLayout();
    DLabel *pTitleLable = new DLabel();
    pTitleLable->setAlignment(Qt::AlignCenter);
    DDialogCloseButton *pCloseBt = new DDialogCloseButton();
    connect(pCloseBt, &DDialogCloseButton::clicked, this, [ = ]() {
        reject();
    });
    pHBoxLayout->addWidget(pTitleLable);
    pHBoxLayout->addWidget(pCloseBt);

    QGridLayout *pGridLayout = new QGridLayout();
    DLabel *pNameLabel = new DLabel(tr("Name:"));
    pNameLabel->setFixedWidth(100);
    m_nameLineEdit->lineEdit()->setPlaceholderText(tr("Mandatory"));
    m_nameLineEdit->setStyleSheet("QLineEdit{border:1px solid gray;}");

    DLabel *pCommandLabel = new DLabel(tr("Command:"));
    pCommandLabel->setFixedWidth(100);
    m_commandLineEdit->lineEdit()->setPlaceholderText(tr("Mandatory"));
    m_commandLineEdit->setStyleSheet("QLineEdit{border:1px solid gray;}");
    DLabel *pShortCutsLabel = new DLabel(tr("Shortcuts:"));
    pNameLabel->setFixedWidth(100);

    //pShortCutsLineEdit->setMinimumWidth(100);
    //pShortCutsLineEdit->setStyleSheet("QLineEdit{border:1px solid gray;}");
    m_shortCutsLineEdit->ShortcutDirection(Qt::AlignLeft);
    pGridLayout->addWidget(pNameLabel, 0, 0);
    pGridLayout->addWidget(m_nameLineEdit, 0, 1);
    pGridLayout->addWidget(pCommandLabel, 1, 0);
    pGridLayout->addWidget(m_commandLineEdit, 1, 1);
    pGridLayout->addWidget(pShortCutsLabel, 2, 0);
    pGridLayout->addWidget(m_shortCutsLineEdit, 2, 1);

    QVBoxLayout *pVBoxLayout = new QVBoxLayout();
    pVBoxLayout->addLayout(pHBoxLayout);
    pVBoxLayout->addLayout(pGridLayout);

    DPushButton *pCancelButton = new DPushButton(tr("Cancel"));
    DPushButton *pAddSaveButton = new DPushButton(tr("Add"));
    DPalette pa = DApplicationHelper::instance()->palette(pAddSaveButton);
    QBrush brush = pa.textTips();
    pa.setBrush(DPalette::ButtonText, brush);
    pAddSaveButton->setPalette(pa);

    if (m_type == CCT_ADD) {
        pTitleLable->setText(tr("Add Command"));
    } else {
        pTitleLable->setText(tr("Edit Command"));

        QHBoxLayout *pHboxLayout1 = new QHBoxLayout();
        DCommandLinkButton *pCommandLinkBt = new DCommandLinkButton(tr("Delete Command"));
        //pCommandLinkBt->setStyleSheet("font-color:white;");
        DPalette pa1 = DApplicationHelper::instance()->palette(pCommandLinkBt);
        QBrush brush1 = pa1.textWarning();
        pa1.setBrush(DPalette::ButtonText, brush1);
        pCommandLinkBt->setPalette(pa1);
        //pCommandLinkBt->setFont(QsFont())
        connect(pCommandLinkBt, &DCommandLinkButton::clicked, this, &CustomCommandOptDlg::slotDelCurCustomCommand);
        pHboxLayout1->addStretch();
        pHboxLayout1->addWidget(pCommandLinkBt);
        pHboxLayout1->addStretch();
        pVBoxLayout->addLayout(pHboxLayout1);
        pAddSaveButton->setText(tr("Save"));
    }

    QHBoxLayout *pBtHbLayout = new QHBoxLayout();
    pBtHbLayout->addWidget(pCancelButton);
    pBtHbLayout->addWidget(pAddSaveButton);
    pVBoxLayout->addLayout(pBtHbLayout);
    setLayout(pVBoxLayout);

    connect(pCancelButton, &DPushButton::clicked, this, [ = ]() {
        reject();
    }
           );
    connect(pAddSaveButton, &DPushButton::clicked, this, &CustomCommandOptDlg::slotAddSaveButtonClicked);

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
    if (type == CCT_MODIFY) {
        QString strName = m_action->text();
        QString strCommad = m_action->data().toString();
        QKeySequence keyseq  = m_action->shortcut();
        m_nameLineEdit->setText(strName);
        m_commandLineEdit->setText(strCommad);
        m_shortCutsLineEdit->setKeySequence(keyseq);
    }
    Dtk::Widget::moveToCenter(this);
}

CustomCommandOptDlg::~CustomCommandOptDlg()
{

}

void CustomCommandOptDlg::slotAddSaveButtonClicked()
{
    QString strName = m_nameLineEdit->text();
    QString strCommand = m_commandLineEdit->text();
    QKeySequence keytmp = m_shortCutsLineEdit->keySequence();

    if (strName.isEmpty() || strCommand.isEmpty()) {
        return ;
    }

    m_newAction.setText(strName);
    m_newAction.setData(strCommand);
    m_newAction.setShortcut(m_shortCutsLineEdit->keySequence());
    accept();
}

void CustomCommandOptDlg::slotDelCurCustomCommand()
{
    m_bDelOpt = true;
    reject();
}
