#include "serverconfigitem.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "serverconfigmanager.h"
ServerConfigItem::ServerConfigItem(ServerConfig *config, bool bGroup, const QString &group, QWidget *parent) : QWidget(parent)
    , m_serverConfig(config)
    , m_nameLabel(new DLabel(this))
    , m_detailsLabel(new DLabel(this))
    //, m_rightIconButton(new MyIconButton(this))
    , m_leftIcon(new DLabel(this))
    //, m_rightIcon(new DLabel(this))
    , m_bGroup(bGroup)
    , m_strGroupName(group)

{
    QImage img(":/images/icon/hover/circlebutton_add _hover.svg");
    m_leftIcon->setPixmap(QPixmap::fromImage(img));
    m_leftIcon->setFixedSize(QSize(70, 70));
    m_leftIcon->setStyleSheet("border:none;");
    m_leftIcon->setFocusPolicy(Qt::NoFocus);
    if (!m_bGroup) {
        m_rightIconButton = new MyIconButton(this);
        m_rightIconButton->setFixedSize(QSize(70, 70));
        m_rightIconButton->setStyleSheet("border:none;");
        m_rightIconButton->setIcon(QIcon(":/images/icon/hover/circlebutton_add _hover.svg"));
        connect(m_rightIconButton, &DIconButton::clicked, this, &ServerConfigItem::editServerConfig);
        m_rightIcon = nullptr;
    } else {
        m_rightIcon = new DLabel(this);
        m_rightIcon->setPixmap(QPixmap::fromImage(img));
        m_rightIcon->setFixedSize(QSize(70, 70));
        m_rightIcon->setStyleSheet("border:none;");
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
        mainLayout->addWidget(m_rightIcon);
    } else {
        mainLayout->addWidget(m_rightIconButton);

    }
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);
    setLayout(mainLayout);

    if (m_bGroup) {
        QMap<QString, QList<ServerConfig *> > &configMap = ServerConfigManager::instance()->getServerCommands();
        m_nameLabel->setText(group);
        m_detailsLabel->setText(QString("%1 server").arg(configMap[group].count()));
    } else {
        m_rightIconButton->hide();
        if (m_serverConfig) {
            m_nameLabel->setText(m_serverConfig->m_serverName);
            m_detailsLabel->setText(QString("%1@%2@%3").arg(m_serverConfig->m_userName).arg(m_serverConfig->m_address).arg(m_serverConfig->m_port));
        }
    }
}

ServerConfig *ServerConfigItem::getCurServerConfig()
{
    return  m_serverConfig;
}

void ServerConfigItem::editServerConfig()
{
    emit modifyServerConfig(this);
}

void ServerConfigItem::enterEvent(QEvent *event)
{
    if (!m_bGroup && m_rightIconButton)
        m_rightIconButton->show();
    DWidget::enterEvent(event);
}
void ServerConfigItem::leaveEvent(QEvent *event)
{
    if (!m_bGroup && m_rightIconButton)
        m_rightIconButton->hide();
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
