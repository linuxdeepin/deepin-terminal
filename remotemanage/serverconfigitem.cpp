#include "serverconfigitem.h"
#include "serverconfigmanager.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <DApplicationHelper>

ServerConfigItem::ServerConfigItem(ServerConfig *config, bool bGroup, const QString &group, QWidget *parent)
    : DWidget(parent),
      m_serverConfig(config),
      m_nameLabel(new DLabel(this)),
      m_detailsLabel(new DLabel(this)),
      m_leftIcon(new DLabel(this)),
      m_bGroup(bGroup),
      m_strGroupName(group)

{
    setFixedWidth(230);
    QImage img(":/resources/images/icon/hover/server.svg");
    m_leftIcon->setPixmap(QPixmap::fromImage(img));
    m_leftIcon->setFixedSize(QSize(70, 70));
    m_leftIcon->setFocusPolicy(Qt::NoFocus);
    m_leftIcon->setContentsMargins(10, 0, 0, 0);
    if (!m_bGroup) {
        m_rightIconButton = new MyIconButton(this);
        m_rightIconButton->setFixedSize(QSize(30, 30));
        m_rightIconButton->setIconSize(QSize(30, 30));
        m_rightIconButton->setIcon(QIcon(":/resources/images/icon/hover/edit_hover.svg"));
        connect(m_rightIconButton, &DIconButton::clicked, this, &ServerConfigItem::editServerConfig);
        m_rightIcon = nullptr;
    } else {
        m_rightIcon = new DLabel(this);
        m_rightIcon->setPixmap(QPixmap::fromImage(QImage(":/resources/images/icon/hover/arrowr.svg")));
        m_rightIcon->setFixedSize(QSize(30, 30));
        m_rightIcon->setAlignment(Qt::AlignCenter);
        m_rightIcon->setFocusPolicy(Qt::NoFocus);
        m_rightIconButton = nullptr;
    }

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addWidget(m_nameLabel);
    vLayout->addWidget(m_detailsLabel);
    vLayout->setSpacing(0);
    vLayout->setMargin(0);
    QHBoxLayout *mainLayout = new QHBoxLayout();
    mainLayout->addWidget(m_leftIcon);
    mainLayout->addLayout(vLayout);
    if (m_bGroup) {
        mainLayout->addWidget(m_rightIcon, 0, Qt::AlignRight);
    } else {
        mainLayout->addWidget(m_rightIconButton);
    }
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 20, 0);
    setLayout(mainLayout);

    if (m_bGroup) {
        QMap<QString, QList<ServerConfig *>> &configMap = ServerConfigManager::instance()->getServerConfigs();
        m_nameLabel->setText(group);
        m_detailsLabel->setText(QString("%1 server").arg(configMap[group].count()));
    } else {
        m_rightIconButton->hide();
        if (m_serverConfig) {
            m_nameLabel->setText(m_serverConfig->m_serverName);
            m_detailsLabel->setText(QString("%1@%2@%3")
                                    .arg(m_serverConfig->m_userName)
                                    .arg(m_serverConfig->m_address)
                                    .arg(m_serverConfig->m_port));
        }
    }
}

ServerConfig *ServerConfigItem::getCurServerConfig()
{
    return m_serverConfig;
}

void ServerConfigItem::editServerConfig()
{
    emit modifyServerConfig(this);
}

void ServerConfigItem::enterEvent(QEvent *event)
{
    if (!m_bGroup && m_rightIconButton) {
        m_rightIconButton->show();
    }
    this->setAutoFillBackground(true);
    this->setBackgroundRole(DPalette::ColorRole::Light);
    DWidget::enterEvent(event);
}

void ServerConfigItem::leaveEvent(QEvent *event)
{
    if (!m_bGroup && m_rightIconButton) {
        m_rightIconButton->hide();
    }
    this->setAutoFillBackground(true);
    this->setBackgroundRole(DPalette::ColorRole::NoRole);
    DWidget::enterEvent(event);
}

bool ServerConfigItem::isGroup()
{
    return m_bGroup;
}

QString ServerConfigItem::getGroupName()
{
    return m_strGroupName;
}
