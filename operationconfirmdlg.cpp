#include "operationconfirmdlg.h"
#include <DButtonBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <DIconButton>
#include <DApplicationHelper>
OperationConfirmDlg::OperationConfirmDlg(QWidget *parent) : DAbstractDialog(parent)
    , m_iconLabel(new DLabel(this))
    , m_operatType(new DLabel(this))
    , m_tipInfo(new DLabel(this))
    , m_okButton(new DButtonBoxButton(tr("Delete")))
    , m_cancelButton(new DButtonBoxButton(tr("Cancel")))
{
    //setWindowFlag(Qt::Dialog| Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
    //setAttribute(Qt::WA_ShowModal, true);

    //m_iconLabel->setPixmap(QPixmap(QIcon::fromTheme("deepin-terminal")));
    setWindowModality(Qt::ApplicationModal);
    setFixedSize(300, 200);
    QHBoxLayout *headLayout = new QHBoxLayout();
    DIconButton *pCloseButton = new DIconButton(QStyle::SP_DialogCloseButton);
    headLayout->addStretch();
    headLayout->addWidget(pCloseButton);
    headLayout->setSpacing(0);
    headLayout->setMargin(0);
    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addStretch();
    vLayout->addWidget(m_operatType);
    vLayout->addWidget(m_tipInfo);
    vLayout->addStretch();
    vLayout->setSpacing(0);
    vLayout->setMargin(0);
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addWidget(m_iconLabel);
    hLayout->addLayout(vLayout);
    hLayout->setSpacing(0);
    hLayout->setMargin(0);
    DButtonBox *btBox = new DButtonBox();
    //m_okButton->setText(tr("Ok"));
    //m_cancelButton->setText(tr("Cancel"));
    DPalette pb = DApplicationHelper::instance()->palette(m_okButton);
    pb.setBrush(DPalette::Base, pb.color(DPalette::ItemBackground));
    pb.setColor(QPalette::Text, QColor(255, 0, 0));
    m_okButton->setPalette(pb);
    btBox->setFixedHeight(50);
    QList<DButtonBoxButton *> listBtnBox;
    listBtnBox.append(m_cancelButton);
    listBtnBox.append(m_okButton);
    //m_okButton->setFocus(Qt::StrongFocus);
    //m_okButton->setStyleSheet("hover{background-color:blue}");
    //m_cancelButton->setStyleSheet("hover{background-color:blue}");
    btBox->setButtonList(listBtnBox, true);
#if 0
    QHBoxLayout *ptmp = new QHBoxLayout();
    DSuggestButton *pok = new DSuggestButton(tr("ok"));
    DSuggestButton *pcancel = new DSuggestButton(tr("cancel"));
    ptmp->addWidget(pcancel);
    ptmp->addWidget(pok);
    ptmp->setSpacing(0);
    ptmp->setMargin(0);
#endif
    QVBoxLayout *pVLayout1 = new QVBoxLayout();
    pVLayout1->addLayout(headLayout);
    pVLayout1->addLayout(hLayout);
    //pVLayout1->addLayout(ptmp);
    pVLayout1->addWidget(btBox);
    setLayout(pVLayout1);
    pVLayout1->setSpacing(0);
    pVLayout1->setMargin(0);
    connect(btBox, &DButtonBox::buttonClicked, this, [ = ](QAbstractButton * bt) {
        if (bt == m_okButton) {
            m_confirmResultCode = QDialog::Accepted;
            close();
        } else {
            m_confirmResultCode = QDialog::Rejected;
            close();
        }
    });
    connect(pCloseButton, &DIconButton::clicked, this, [ = ]() {
        m_confirmResultCode = QDialog::Rejected;
        close();
    });
    //connect(m_okButton, &DButtonBoxButton::clicked, this, &OperationConfirmDlg::slotClickOkButton);
    //connect(m_cancelButton, &DButtonBoxButton::clicked, this, &OperationConfirmDlg::slotClickCancelButton);
}

void OperationConfirmDlg::setOperatTypeName(const QString &strName)
{
    m_operatType->setText(strName);
}
void OperationConfirmDlg::setTipInfo(const QString &strInfo)
{
    m_tipInfo->setText(strInfo);
}

void OperationConfirmDlg::slotClickOkButton()
{
    m_confirmResultCode = QDialog::Accepted;
    close();
}
void OperationConfirmDlg::slotClickCancelButton()
{
    m_confirmResultCode = QDialog::Rejected;
    close();
}
QDialog::DialogCode OperationConfirmDlg::getConfirmResult()
{
    return m_confirmResultCode;
}
