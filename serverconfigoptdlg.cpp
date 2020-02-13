#include "serverconfigoptdlg.h"
#include "serverconfigmanager.h"

#include <DFontSizeManager>
#include <DPushButton>
#include <DSuggestButton>

#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTextCodec>

ServerConfigOptDlg::ServerConfigOptDlg(ServerConfigOptType type, ServerConfig *curServer, QWidget *parent)
    : DAbstractDialog(parent),
      m_type(type),
      m_curServer(curServer),
      m_titleLabel(new DLabel),
      m_closeButton(new DDialogCloseButton()),
      m_serverName(new DLineEdit),
      m_address(new DLineEdit),
      m_port(new DSpinBox),
      m_userName(new DLineEdit),
      m_password(new DPasswordEdit),
      m_privateKey(new DFileChooserEdit),
      m_group(new DLineEdit),
      m_path(new DLineEdit),
      m_command(new DLineEdit),
      m_coding(new DComboBox),
      m_backSapceKey(new DComboBox),
      m_deleteKey(new DComboBox),
      m_advancedOptions(new DCommandLinkButton(tr("Advanced options"))),
      m_delServer(new DCommandLinkButton(tr("Delete server")))

{
    setWindowModality(Qt::ApplicationModal);
    setFixedWidth(420);
    setAutoFillBackground(true);
    initUI();
    initData();
}

void ServerConfigOptDlg::initUI()
{
    //整个垂直布局
    QVBoxLayout *m_VBoxLayout = new QVBoxLayout();
    //头部布局
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setText(tr("Add Server"));
    DFontSizeManager::instance()->bind(m_titleLabel, DFontSizeManager::T5);
    QHBoxLayout *headLayout = new QHBoxLayout();
    headLayout->addWidget(m_titleLabel);
    headLayout->addWidget(m_closeButton);
    headLayout->setSpacing(0);
    headLayout->setMargin(0);
    headLayout->setAlignment(Qt::AlignTop);
    connect(m_closeButton, &DDialogCloseButton::clicked, this, [=]() { reject(); });
    m_VBoxLayout->addLayout(headLayout);

    //数据部分
    QGridLayout *pGridLayout = new QGridLayout();
    pGridLayout->setColumnStretch(1, 1);
    pGridLayout->setContentsMargins(10, 10, 10, 0);
    DLabel *pServerNameLabel = new DLabel(tr("Server name:"));
    pServerNameLabel->setAlignment(Qt::AlignLeft);
    pServerNameLabel->setAlignment(Qt::AlignVCenter);
    pServerNameLabel->setFixedWidth(100);
    pServerNameLabel->setFixedHeight(30);
    m_serverName->lineEdit()->setPlaceholderText(tr("Optional"));
    pGridLayout->addWidget(pServerNameLabel);
    pGridLayout->addWidget(m_serverName);

    DLabel *pAddressLabel = new DLabel(tr("Address:"));
    pAddressLabel->setAlignment(Qt::AlignLeft);
    pAddressLabel->setAlignment(Qt::AlignVCenter);
    pAddressLabel->setFixedWidth(100);
    pAddressLabel->setFixedHeight(30);
    m_address->lineEdit()->setPlaceholderText(tr("Optional"));
    m_address->setFixedWidth(150);
    pGridLayout->addWidget(pAddressLabel);
    pGridLayout->addWidget(m_address);

    QHBoxLayout *portLayout = new QHBoxLayout();
    DLabel *pPortLabel = new DLabel(tr("Port:"));
    pPortLabel->setAlignment(Qt::AlignLeft);
    pPortLabel->setAlignment(Qt::AlignVCenter);
    pPortLabel->setFixedWidth(40);
    m_port->setRange(0, 65535);
    m_port->setValue(22);
    m_port->setSingleStep(1);
    m_port->setFixedWidth(70);
    portLayout->addWidget(pPortLabel);
    portLayout->addWidget(m_port);
    pGridLayout->addLayout(portLayout, 1, 1, Qt::AlignRight);

    DLabel *pUsernameLabel = new DLabel(tr("Username:"));
    pUsernameLabel->setAlignment(Qt::AlignLeft);
    pUsernameLabel->setAlignment(Qt::AlignVCenter);
    pUsernameLabel->setFixedWidth(100);
    pUsernameLabel->setFixedHeight(30);
    m_userName->lineEdit()->setPlaceholderText(tr("Optional"));
    pGridLayout->addWidget(pUsernameLabel);
    pGridLayout->addWidget(m_userName);

    DLabel *pPasswordLabel = new DLabel(tr("Password:"));
    pPasswordLabel->setAlignment(Qt::AlignLeft);
    pPasswordLabel->setAlignment(Qt::AlignVCenter);
    pPasswordLabel->setFixedWidth(100);
    pPasswordLabel->setFixedHeight(30);
    pGridLayout->addWidget(pPasswordLabel);
    pGridLayout->addWidget(m_password);

    DLabel *pPrivateKeyLabel = new DLabel(tr("PrivateKey:"));
    pPrivateKeyLabel->setAlignment(Qt::AlignLeft);
    pPrivateKeyLabel->setAlignment(Qt::AlignVCenter);
    pPrivateKeyLabel->setFixedWidth(100);
    pPrivateKeyLabel->setFixedHeight(30);
    pGridLayout->addWidget(pPrivateKeyLabel);
    pGridLayout->addWidget(m_privateKey);

    //高级选项
    DWidget *seniorWidget = new DWidget;
    QGridLayout *seniorLayout = new QGridLayout();
    seniorLayout->setAlignment(Qt::AlignTop);
    seniorLayout->setContentsMargins(10, 0, 10, 10);
    seniorLayout->setColumnStretch(1, 1);
    DLabel *pGroupLabel = new DLabel(tr("Group:"));
    pGroupLabel->setAlignment(Qt::AlignLeft);
    pGroupLabel->setAlignment(Qt::AlignVCenter);
    pGroupLabel->setFixedWidth(100);
    pGroupLabel->setFixedHeight(30);
    seniorLayout->addWidget(pGroupLabel);
    seniorLayout->addWidget(m_group);

    DLabel *pPathLabel = new DLabel(tr("Path:"));
    pPathLabel->setAlignment(Qt::AlignLeft);
    pPathLabel->setAlignment(Qt::AlignVCenter);
    pPathLabel->setFixedWidth(100);
    pPathLabel->setFixedHeight(30);
    seniorLayout->addWidget(pPathLabel);
    seniorLayout->addWidget(m_path);

    DLabel *pCommandLabel = new DLabel(tr("Command:"));
    pCommandLabel->setAlignment(Qt::AlignLeft);
    pCommandLabel->setAlignment(Qt::AlignVCenter);
    pCommandLabel->setFixedWidth(100);
    pCommandLabel->setFixedHeight(30);
    seniorLayout->addWidget(pCommandLabel);
    seniorLayout->addWidget(m_command);

    DLabel *pCodingLabel = new DLabel(tr("Coding:"));
    pCodingLabel->setAlignment(Qt::AlignLeft);
    pCodingLabel->setAlignment(Qt::AlignVCenter);
    pCodingLabel->setFixedWidth(100);
    pCodingLabel->setFixedHeight(30);
    seniorLayout->addWidget(pCodingLabel);
    seniorLayout->addWidget(m_coding);

    DLabel *pBackspaceKeyLabel = new DLabel(tr("BackspaceKey:"));
    pBackspaceKeyLabel->setAlignment(Qt::AlignLeft);
    pBackspaceKeyLabel->setAlignment(Qt::AlignVCenter);
    pBackspaceKeyLabel->setFixedWidth(100);
    pBackspaceKeyLabel->setFixedHeight(30);
    seniorLayout->addWidget(pBackspaceKeyLabel);
    seniorLayout->addWidget(m_backSapceKey);

    DLabel *pDeleteKeyLabel = new DLabel(tr("DeleteKey:"));
    pDeleteKeyLabel->setAlignment(Qt::AlignLeft);
    pDeleteKeyLabel->setAlignment(Qt::AlignVCenter);
    pDeleteKeyLabel->setFixedWidth(100);
    pDeleteKeyLabel->setFixedHeight(30);
    seniorLayout->addWidget(pDeleteKeyLabel);
    seniorLayout->addWidget(m_deleteKey);
    seniorWidget->setLayout(seniorLayout);

    connect(m_advancedOptions, &DCommandLinkButton::clicked, this, [=]() {
        m_advancedOptions->hide();
        seniorWidget->show();
        if (m_type == SCT_MODIFY) {
            m_delServer->show();
        }
    });

    m_VBoxLayout->addLayout(pGridLayout);
    m_VBoxLayout->addWidget(m_advancedOptions, 0, Qt::AlignHCenter);
    m_VBoxLayout->addWidget(seniorWidget);
    m_VBoxLayout->addWidget(m_delServer, 0, Qt::AlignHCenter);
    m_delServer->hide();
    seniorWidget->hide();

    DPushButton *pCancelButton = new DPushButton(tr("Cancel"));
    DSuggestButton *pAddSaveButton = new DSuggestButton(tr("Add"));
    if (m_type == SCT_MODIFY) {
        pAddSaveButton->setText(tr("Save"));
    }
    DPalette pa = DApplicationHelper::instance()->palette(pAddSaveButton);
    QBrush brush = pa.textLively().color();
    pa.setBrush(DPalette::ButtonText, brush);
    pAddSaveButton->setPalette(pa);
    QHBoxLayout *pBtHbLayout = new QHBoxLayout();
    pBtHbLayout->addWidget(pCancelButton);
    pBtHbLayout->addWidget(pAddSaveButton);
    m_VBoxLayout->addLayout(pBtHbLayout);

    setLayout(m_VBoxLayout);

    connect(pCancelButton, &DPushButton::clicked, this, [=]() { reject(); });
    connect(pAddSaveButton, &DPushButton::clicked, this, &ServerConfigOptDlg::slotAddSaveButtonClicked);
    connect(m_delServer, &DCommandLinkButton::clicked, this, [=]() {
        setDelServer(true);
        accept();
    });
}

void ServerConfigOptDlg::initData()
{
    QList<QString> textCodeList = getTextCodec();
    m_coding->addItems(textCodeList);
    QList<QString> backSpaceKeyList = getBackSpaceKey();
    m_backSapceKey->addItems(backSpaceKeyList);
    QList<QString> deleteKeyList = getDeleteKey();
    m_deleteKey->addItems(deleteKeyList);
    if (m_type == SCT_MODIFY && m_curServer != nullptr) {
        m_serverName->setText(m_curServer->m_serverName);
        m_address->setText(m_curServer->m_address);
        m_port->setValue(m_curServer->m_port.toInt());
        m_userName->setText(m_curServer->m_userName);
        m_password->setText(m_curServer->m_password);
        m_privateKey->setText(m_curServer->m_privateKey);
        m_group->setText(m_curServer->m_group);
        m_path->setText(m_curServer->m_path);
        m_command->setText(m_curServer->m_command);
        if (!m_curServer->m_encoding.isEmpty()) {
            int textCodeIndex = textCodeList.indexOf(m_curServer->m_encoding);
            m_coding->setCurrentIndex(textCodeIndex);
        }
        if (!m_curServer->m_backspaceKey.isEmpty()) {
            int backSpaceKeyIndex = backSpaceKeyList.indexOf(m_curServer->m_backspaceKey);
            m_backSapceKey->setCurrentIndex(backSpaceKeyIndex);
        }
        if (!m_curServer->m_deleteKey.isEmpty()) {
            int deleteKeyIndex = deleteKeyList.indexOf(m_curServer->m_deleteKey);
            m_deleteKey->setCurrentIndex(deleteKeyIndex);
        }
    }
}

QList<QString> ServerConfigOptDlg::getTextCodec()
{
    QList<QByteArray> list = QTextCodec::availableCodecs();
    QList<QString> textCodecList;
    for (QByteArray byteArr : list) {
        QString str = QString(byteArr);
        if (!textCodecList.contains(str)) {
            textCodecList.append(str);
        }
    }
    return textCodecList;
}

QList<QString> ServerConfigOptDlg::getBackSpaceKey()
{
    QList<QString> eraseKeyList;
    eraseKeyList.append(tr("ascii-del"));
    eraseKeyList.append(tr("auto"));
    eraseKeyList.append(tr("control-h"));
    eraseKeyList.append(tr("escape-sequence"));
    eraseKeyList.append(tr("ttys"));
    return eraseKeyList;
}

QList<QString> ServerConfigOptDlg::getDeleteKey()
{
    QList<QString> eraseKeyList;
    eraseKeyList.append(tr("escape-sequence"));
    eraseKeyList.append(tr("ascii-del"));
    eraseKeyList.append(tr("auto"));
    eraseKeyList.append(tr("control-h"));
    eraseKeyList.append(tr("ttys"));
    return eraseKeyList;
}

ServerConfigOptDlg::~ServerConfigOptDlg()
{
}

void ServerConfigOptDlg::slotAddSaveButtonClicked()
{
    ServerConfig *config = new ServerConfig();
    config->m_serverName = m_serverName->text();
    config->m_address = m_address->text();
    config->m_userName = m_userName->text();
    config->m_password = m_password->text();
    config->m_privateKey = m_privateKey->text();
    config->m_port = m_port->text();
    config->m_group = m_group->text();
    config->m_path = m_path->text();
    config->m_command = m_command->text();
    config->m_encoding = m_coding->currentText();
    config->m_backspaceKey = m_backSapceKey->currentText();
    config->m_deleteKey = m_deleteKey->currentText();
    if (m_type == SCT_ADD) {
        ServerConfigManager::instance()->saveServerConfig(config);
    } else if (m_type == SCT_MODIFY && m_curServer != nullptr) {
        ServerConfigManager::instance()->modifyServerConfig(config, m_curServer);
    }

    accept();
}

void ServerConfigOptDlg::slotClose()
{
    reject();
}
