#include "serverconfigoptdlg.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
ServerConfigOptDlg::ServerConfigOptDlg(ServerConfigOptType type, QAction *curAction, QWidget *parent) : DAbstractDialog(parent)
    , m_titleLabel(new DLabel)
    , m_closeButton(new DDialogCloseButton())
    , m_serverName(new DLineEdit)
    , m_address(new DLineEdit)
    , m_port(new DSpinBox)
    , m_userName(new DLineEdit)
    , m_password(new DPasswordEdit)
    , m_privateKey(new DFileChooserEdit)
    , m_group(new DLineEdit)
    , m_path(new DLineEdit)
    , m_command(new DLineEdit)
    , m_coding(new DComboBox)
    , m_backSapceKey(new DComboBox)
    , m_deleteKey(new DComboBox)
    , m_advancedOptions(new DCommandLinkButton(tr("Advanced options")))
    , m_delServer(new DCommandLinkButton(tr("Delete server")))

{
    setWindowModality(Qt::ApplicationModal);
    setFixedSize(400, 500);
    //整个垂直布局
    QVBoxLayout *m_VBoxLayout = new QVBoxLayout();

    //头部布局
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setText(tr("Add Server"));
    QHBoxLayout *headLayout = new QHBoxLayout();
    headLayout->addWidget(m_titleLabel);
    headLayout->addWidget(m_closeButton);
    headLayout->setSpacing(0);
    headLayout->setMargin(0);
    headLayout->setAlignment(Qt::AlignTop);
    connect(m_closeButton, &DDialogCloseButton::clicked, this, [ = ]() {
        reject();
    });
    m_VBoxLayout->addLayout(headLayout);

    //数据部分
    QGridLayout *pGridLayout = new QGridLayout();
    pGridLayout->setAlignment(Qt::AlignTop);
    pGridLayout->setColumnStretch(1,1);
    DLabel *pServerNameLabel = new DLabel(tr("Server name:"));
    pServerNameLabel->setAlignment(Qt::AlignLeft);
    pServerNameLabel->setFixedWidth(100);
    pServerNameLabel->setFixedHeight(30);
    m_serverName->lineEdit()->setPlaceholderText(tr("Optional"));
    pGridLayout->addWidget(pServerNameLabel);
    pGridLayout->addWidget(m_serverName);

    DLabel *pAddressLabel = new DLabel(tr("Address:"));
    pAddressLabel->setAlignment(Qt::AlignLeft);
    pAddressLabel->setFixedWidth(100);
    pAddressLabel->setFixedHeight(30);
    m_address->lineEdit()->setPlaceholderText(tr("Optional"));
    pGridLayout->addWidget(pAddressLabel);
    pGridLayout->addWidget(m_address);

    DLabel *pPortLabel = new DLabel(tr("Port:"));
    pPortLabel->setAlignment(Qt::AlignLeft);
    pPortLabel->setFixedWidth(40);
    m_port->setRange(0, 65535);
    m_port->setValue(22);
    m_port->setSingleStep(1);
    pGridLayout->addWidget(pPortLabel);
    pGridLayout->addWidget(m_port);

    DLabel *pUsernameLabel = new DLabel(tr("Username:"));
    pUsernameLabel->setAlignment(Qt::AlignLeft);
    pUsernameLabel->setFixedWidth(100);
    pUsernameLabel->setFixedHeight(30);
    m_userName->lineEdit()->setPlaceholderText(tr("Optional"));
    pGridLayout->addWidget(pUsernameLabel);
    pGridLayout->addWidget(m_userName);

    DLabel *pPasswordLabel = new DLabel(tr("Password:"));
    pPasswordLabel->setAlignment(Qt::AlignLeft);
    pPasswordLabel->setFixedWidth(100);
    pPasswordLabel->setFixedHeight(30);
    pGridLayout->addWidget(pPasswordLabel);
    pGridLayout->addWidget(m_password);

    DLabel *pPrivateKeyLabel = new DLabel(tr("PrivateKey:"));
    pPrivateKeyLabel->setAlignment(Qt::AlignLeft);
    pPrivateKeyLabel->setFixedWidth(100);
    pPrivateKeyLabel->setFixedHeight(30);
    pGridLayout->addWidget(pPrivateKeyLabel);
    pGridLayout->addWidget(m_privateKey);

    DLabel *pGroupLabel = new DLabel(tr("Group:"));
    pGroupLabel->setAlignment(Qt::AlignLeft);
    pGroupLabel->setFixedWidth(100);
    pGroupLabel->setFixedHeight(30);
    pGridLayout->addWidget(pGroupLabel);
    pGridLayout->addWidget(m_group);

    DLabel *pPathLabel = new DLabel(tr("Path:"));
    pPathLabel->setAlignment(Qt::AlignLeft);
    pPathLabel->setFixedWidth(100);
    pPathLabel->setFixedHeight(30);
    pGridLayout->addWidget(pPathLabel);
    pGridLayout->addWidget(m_path);

    DLabel *pCommandLabel = new DLabel(tr("Command:"));
    pCommandLabel->setAlignment(Qt::AlignLeft);
    pCommandLabel->setFixedWidth(100);
    pCommandLabel->setFixedHeight(30);
    pGridLayout->addWidget(pCommandLabel);
    pGridLayout->addWidget(m_command);

    DLabel *pCodingLabel = new DLabel(tr("Coding:"));
    pCodingLabel->setAlignment(Qt::AlignLeft);
    pCodingLabel->setFixedWidth(100);
    pCodingLabel->setFixedHeight(30);
    pGridLayout->addWidget(pCodingLabel);
    pGridLayout->addWidget(m_coding);

    DLabel *pBackspaceKeyLabel = new DLabel(tr("BackspaceKey:"));
    pBackspaceKeyLabel->setAlignment(Qt::AlignLeft);
    pBackspaceKeyLabel->setFixedWidth(100);
    pBackspaceKeyLabel->setFixedHeight(30);
    pGridLayout->addWidget(pBackspaceKeyLabel);
    pGridLayout->addWidget(m_backSapceKey);

    DLabel *pDeleteKeyLabel = new DLabel(tr("DeleteKey:"));
    pDeleteKeyLabel->setAlignment(Qt::AlignLeft);
    pDeleteKeyLabel->setFixedWidth(100);
    pDeleteKeyLabel->setFixedHeight(30);
    pGridLayout->addWidget(pDeleteKeyLabel);
    pGridLayout->addWidget(m_deleteKey);

    m_VBoxLayout->addLayout(pGridLayout);
    setLayout(m_VBoxLayout);
}

ServerConfigOptDlg::~ServerConfigOptDlg()
{

}

void ServerConfigOptDlg::slotClose()
{

}
