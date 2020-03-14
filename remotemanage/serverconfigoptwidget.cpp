#include "serverconfigoptwidget.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

ServerConfigOptWidget::ServerConfigOptWidget(QWidget *parent)
    : QWidget(parent),
      m_titleLabel(new DLabel),
      m_closeButton(new DIconButton(QStyle::SP_DialogCloseButton)),
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
    setFixedSize(300, 500);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    QHBoxLayout *headLayout = new QHBoxLayout();
    headLayout->addStretch();
    headLayout->addWidget(m_titleLabel);
    headLayout->addStretch();
    headLayout->addWidget(m_closeButton);
    headLayout->setSpacing(0);
    headLayout->setMargin(0);

    QGridLayout *pGridLayout = new QGridLayout();
    DLabel *pServerNameLabel = new DLabel(tr("Server name:"));
    pServerNameLabel->setAlignment(Qt::AlignLeft);
    pServerNameLabel->setFixedWidth(100);
    m_serverName->lineEdit()->setPlaceholderText(tr("Optional"));

    DLabel *pAddressLabel = new DLabel(tr("Address:"));
    pAddressLabel->setAlignment(Qt::AlignLeft);
    pAddressLabel->setFixedWidth(100);
    m_address->lineEdit()->setPlaceholderText(tr("Optional"));

    DLabel *pPortLabel = new DLabel(tr("Port:"));
    pPortLabel->setAlignment(Qt::AlignLeft);
    pPortLabel->setFixedWidth(40);
    m_port->setRange(0, 65535);
    m_port->setValue(22);
    m_port->setSingleStep(1);

    DLabel *pUsernameLabel = new DLabel(tr("Username:"));
    pPortLabel->setAlignment(Qt::AlignLeft);
    pPortLabel->setFixedWidth(100);
    m_userName->lineEdit()->setPlaceholderText(tr("Optional"));

    connect(m_closeButton, &DIconButton::clicked, this, &ServerConfigOptWidget::slotClose);
}

void ServerConfigOptWidget::slotClose()
{
}
