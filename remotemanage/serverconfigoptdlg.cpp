#include "serverconfigoptdlg.h"
#include "serverconfigmanager.h"
#include "termcommandlinkbutton.h"
#include "../views/operationconfirmdlg.h"
#include "utils.h"

#include <DFontSizeManager>
#include <DPushButton>
#include <DSuggestButton>
#include <DVerticalLine>
#include <DApplicationHelper>

#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTextCodec>
#include <QSpacerItem>
#include <iterator>//added byq qinyaning
#include <QDebug>

ServerConfigOptDlg::ServerConfigOptDlg(ServerConfigOptType type, ServerConfig *curServer, QWidget *parent)
    : DAbstractDialog(parent),
      m_type(type),
      m_curServer(curServer),
      m_iconLabel(new DLabel),
      m_titleLabel(new DLabel),
      m_closeButton(new DWindowCloseButton(this)),
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
      m_delServer(new TermCommandLinkButton())

{
    qDebug() << "ServerConfigOptDlg init";
    setWindowModality(Qt::ApplicationModal);
    setFixedWidth(459);
    setAutoFillBackground(true);
    initUI();
    initData();
    qDebug() << "ServerConfigOptDlg init finish";
}

void ServerConfigOptDlg::initUI()
{
    qDebug() << "ServerConfigOptDlg init UI";
    //all layout
    QVBoxLayout *m_VBoxLayout = new QVBoxLayout();
    m_VBoxLayout->setSpacing(10);
    m_VBoxLayout->setContentsMargins(0, 0, 0, 10);
    this->setFixedHeight(392);
    //head layout
    m_iconLabel->setFixedSize(36, 34);
    m_titleLabel->setText(tr("Add Server"));
    m_titleLabel->setFixedWidth(85);

    m_closeButton->setFocusPolicy(Qt::NoFocus);
    m_closeButton->setFixedWidth(50);
    m_closeButton->setIconSize(QSize(50, 50));

    // 字体
    DFontSizeManager::instance()->bind(m_titleLabel, DFontSizeManager::T5, QFont::DemiBold);
    // 字色
    DPalette palette = m_titleLabel->palette();
    palette.setBrush(QPalette::WindowText, palette.color(DPalette::TextTitle));
    m_titleLabel->setPalette(palette);

    QHBoxLayout *headLayout = new QHBoxLayout();
    headLayout->setSpacing(0);
    headLayout->setContentsMargins(0, 0, 0, 0);
    // 为了和closebutton对应是标题居中
    headLayout->addWidget(m_iconLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
    headLayout->addWidget(m_titleLabel, 0, Qt::AlignVCenter );
    headLayout->addWidget(m_closeButton, 0, Qt::AlignRight | Qt::AlignTop);
    connect(m_closeButton, &DDialogCloseButton::clicked, this, [ = ]() {
        reject();
    });
    m_VBoxLayout->addLayout(headLayout, Qt::AlignTop);
    m_VBoxLayout->addStretch(10);
    //main layout
    QGridLayout *pGridLayout = new QGridLayout();
    pGridLayout->setColumnStretch(1, 1);
    pGridLayout->setContentsMargins(28, 0, 30, 0);
    pGridLayout->setSpacing(10);
    DLabel *pServerNameLabel = new DLabel(tr("Server name:"));
    setLabelStyle(pServerNameLabel);

    m_serverName->lineEdit()->setPlaceholderText(tr("Required"));
    pGridLayout->addWidget(pServerNameLabel);
    pGridLayout->addWidget(m_serverName);

    DLabel *pAddressLabel = new DLabel(tr("Address:"));
    setLabelStyle(pAddressLabel);
    m_address->lineEdit()->setPlaceholderText(tr("Required"));
    m_address->setFixedWidth(150);
    pGridLayout->addWidget(pAddressLabel);
    pGridLayout->addWidget(m_address);

    QHBoxLayout *portLayout = new QHBoxLayout();
    DLabel *pPortLabel = new DLabel(tr("Port:"));
    pPortLabel->setAlignment(Qt::AlignLeft);
    pPortLabel->setAlignment(Qt::AlignVCenter);
    pPortLabel->setFixedWidth(40);
    DFontSizeManager::instance()->bind(pPortLabel, DFontSizeManager::T6);
    m_port->setRange(0, 65535);
    m_port->setValue(22);
    m_port->setSingleStep(1);
    m_port->setFixedWidth(70);
    m_port->lineEdit()->setClearButtonEnabled(false);
    // 去除上下按钮
    m_port->setButtonSymbols(DSpinBox::NoButtons);
    // 禁用输入法
    m_port->setAttribute(Qt::WA_InputMethodEnabled, false);
    portLayout->addWidget(pPortLabel);
    portLayout->addWidget(m_port);
    pGridLayout->addLayout(portLayout, 1, 1, Qt::AlignRight);

    DLabel *pUsernameLabel = new DLabel(tr("Username:"));
    setLabelStyle(pUsernameLabel);
    m_userName->lineEdit()->setPlaceholderText(tr("Required"));
    pGridLayout->addWidget(pUsernameLabel);
    pGridLayout->addWidget(m_userName);

    DLabel *pPasswordLabel = new DLabel(tr("Password:"));
    setLabelStyle(pPasswordLabel);

    m_password->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, false);
    pGridLayout->addWidget(pPasswordLabel);
    pGridLayout->addWidget(m_password);

    DLabel *pPrivateKeyLabel = new DLabel(tr("Certificate:"));
    setLabelStyle(pPrivateKeyLabel);

    pGridLayout->addWidget(pPrivateKeyLabel);
    pGridLayout->addWidget(m_privateKey);

    //senior layout
    DWidget *seniorWidget = new DWidget;
    seniorWidget->setContentsMargins(0, 0, 0, 0);
    QGridLayout *seniorLayout = new QGridLayout();
    seniorLayout->setAlignment(Qt::AlignTop);
    seniorLayout->setSpacing(10);
    seniorLayout->setContentsMargins(28, 0, 30, 0);
    seniorLayout->setColumnStretch(1, 1);
    DLabel *pGroupLabel = new DLabel(tr("Group:"));
    setLabelStyle(pGroupLabel);
    seniorLayout->addWidget(pGroupLabel);
    seniorLayout->addWidget(m_group);

    DLabel *pPathLabel = new DLabel(tr("Path:"));
    setLabelStyle(pPathLabel);
    seniorLayout->addWidget(pPathLabel);
    seniorLayout->addWidget(m_path);

    DLabel *pCommandLabel = new DLabel(tr("Command:"));
    setLabelStyle(pCommandLabel);
    seniorLayout->addWidget(pCommandLabel);
    m_command->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, false);
    seniorLayout->addWidget(m_command);

    DLabel *pCodingLabel = new DLabel(tr("Encoding:"));
    setLabelStyle(pCodingLabel);
    seniorLayout->addWidget(pCodingLabel);
    seniorLayout->addWidget(m_coding);

    DLabel *pBackspaceKeyLabel = new DLabel(tr("Backspace key:"));
    setLabelStyle(pBackspaceKeyLabel);
    seniorLayout->addWidget(pBackspaceKeyLabel);
    seniorLayout->addWidget(m_backSapceKey);

    DLabel *pDeleteKeyLabel = new DLabel(tr("Delete key:"));
    setLabelStyle(pDeleteKeyLabel);
    seniorLayout->addWidget(pDeleteKeyLabel);
    seniorLayout->addWidget(m_deleteKey);
    seniorWidget->setLayout(seniorLayout);

    m_VBoxLayout->addLayout(pGridLayout, Qt::AlignHCenter);
    QSpacerItem *upItem = new QSpacerItem(this->width(), 10);
    m_VBoxLayout->addSpacerItem(upItem);
    m_VBoxLayout->addWidget(m_advancedOptions, 0, Qt::AlignHCenter);
    QSpacerItem *downItem = new QSpacerItem(this->width(), 10);
    m_VBoxLayout->addSpacerItem(downItem);
    m_VBoxLayout->addWidget(seniorWidget, 0, Qt::AlignHCenter);
    m_delServer->setText(tr("Delete server"));
    m_VBoxLayout->addWidget(m_delServer, 0, Qt::AlignHCenter);
    m_delServer->hide();
    seniorWidget->hide();

    DFontSizeManager::instance()->bind(m_advancedOptions, DFontSizeManager::T8, QFont::Normal);
    connect(m_advancedOptions, &DCommandLinkButton::clicked, this, [ = ]() {
        m_advancedOptions->hide();
        seniorWidget->show();
        upItem->changeSize(this->width(), 0);
        downItem->changeSize(this->width(), 0);
        if (m_type == SCT_MODIFY) {
            m_delServer->show();
            this->setFixedHeight(670);
            qDebug() << "ServerConfigOptDlg init delet show";
        } else {
            this->setFixedHeight(630);
        }
    });

    DPushButton *pCancelButton = new DPushButton(tr("Cancel"));
    DSuggestButton *pAddSaveButton = new DSuggestButton(tr("Add"));
    if (m_type == SCT_MODIFY) {
        m_titleLabel->setText(tr("Edit server"));
        pAddSaveButton->setText(tr("Save"));
    }
    Utils::setSpaceInWord(pCancelButton);
    Utils::setSpaceInWord(pAddSaveButton);
    DPalette pa = DApplicationHelper::instance()->palette(pAddSaveButton);
    QBrush brush = pa.textLively().color();
    pa.setBrush(DPalette::ButtonText, brush);
    pAddSaveButton->setPalette(pa);
    QHBoxLayout *pBtHbLayout = new QHBoxLayout();
    DVerticalLine *line = new DVerticalLine();
    pBtHbLayout->setContentsMargins(10, 0, 10, 0);
    pBtHbLayout->setSpacing(9);
    pBtHbLayout->addWidget(pCancelButton);
    pBtHbLayout->addWidget(line);
    pBtHbLayout->addWidget(pAddSaveButton);
    m_VBoxLayout->addStretch(10);
    m_VBoxLayout->addLayout(pBtHbLayout, Qt::AlignBottom);
    setLayout(m_VBoxLayout);

    connect(pCancelButton, &DPushButton::clicked, this, [ = ]() {
        reject();
    });
    connect(pAddSaveButton, &DPushButton::clicked, this, &ServerConfigOptDlg::slotAddSaveButtonClicked);
    connect(m_delServer, &DCommandLinkButton::clicked, this, [ = ]() {
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

/*******************************************************************************
 1. @函数:    setLabelStyle
 2. @作者:    m000714 戴正文
 3. @日期:    2020-04-16
 4. @说明:    设置label格式
*******************************************************************************/
void ServerConfigOptDlg::setLabelStyle(DLabel *label)
{
    label->setAlignment(Qt::AlignLeft);
    label->setAlignment(Qt::AlignVCenter);
    label->setFixedWidth(100);
    label->setMinimumHeight(36);

    DFontSizeManager::instance()->bind(label, DFontSizeManager::T6);
}

ServerConfigOptDlg::~ServerConfigOptDlg()
{
}

void ServerConfigOptDlg::slotAddSaveButtonClicked()
{
    // 服务器名为空
    if (m_serverName->text().trimmed().isEmpty()) {
        m_serverName->showAlertMessage(tr("Please enter server name!"), m_serverName);
        return;
    }
    // 地址为空
    if (m_address->text().trimmed().isEmpty()) {
        m_address->showAlertMessage(tr("Please enter IP address!"), m_address);
        return;
    }
    // 端口为空
    if (m_port->text().trimmed().isEmpty()) {
        m_address->showAlertMessage(tr("Please enter port!"), m_port);
        return;
    }

    //---added by qinyaning(nyq) to show the tip when username is empty---//
    if (m_userName->text().trimmed().isEmpty()) { //如果用户名为空， 提示用户名为空， 添加失败
        m_userName->showAlertMessage(tr("Please enter user name!"), m_userName);
        return;
    }
    //------------------------------------------------------------------//
    //--added by qinyaning(nyq) to solve the bug 19116: You can create a new remote server with the same name--//
    if((m_type == SCT_ADD)
            || ((m_type == SCT_MODIFY && m_curServer != nullptr)
                && (m_curServer->m_serverName.trimmed() != m_serverName->text().trimmed())))/*此时用户已经在修改模式下修改了服务器名称*/ {
        QMap<QString, QList<ServerConfig *>> severConfigs = ServerConfigManager::instance()->getServerConfigs();
        for(QMap<QString, QList<ServerConfig *>>::iterator iter = severConfigs.begin(); iter != severConfigs.end(); iter++) {
            QList<ServerConfig *> value = iter.value();
            for(int i = 0; i < value.size(); i++) {
                if(value[i]->m_serverName.trimmed() == m_serverName->text().trimmed()) {//服务器名相同
                    OperationConfirmDlg optDlg;
                    QPixmap warnning = QIcon::fromTheme("dialog-warning").pixmap(QSize(32, 32));
                    optDlg.setIconPixmap(warnning);
                    optDlg.setOperatTypeName("deepin-terminal-warning");
                    optDlg.setTipInfo(QObject::tr("Named the same remote server!"));
                    optDlg.setOKCancelBtnText(QObject::tr("Sure"), QObject::tr("Cancel"));
                    optDlg.setFixedSize(380, 160);
                    optDlg.exec();
                    return;
                }
            }
        }
    }
    //-------------------------------------------------------------------//
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
    m_currentServerName = m_serverName->text();
    accept();
}

void ServerConfigOptDlg::slotClose()
{
    reject();
}
