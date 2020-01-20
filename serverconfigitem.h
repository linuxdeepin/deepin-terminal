#ifndef SERVERCONFIGITEM_H
#define SERVERCONFIGITEM_H

#include "myiconbutton.h"
#include "serverconfigmanager.h"
#include <DIconButton>
#include <DLabel>
#include <DWidget>
#include <QAction>
#include <QWidget>

DWIDGET_USE_NAMESPACE
class ServerConfigItem : public QWidget
{
    Q_OBJECT
public:
    explicit ServerConfigItem(ServerConfig *config, bool bGroup = false, const QString &group = "",
                              QWidget *parent = nullptr);
    ServerConfig *getCurServerConfig();
    bool          isGroup();
    QString       getGroupName();
signals:
    void modifyServerConfig(ServerConfigItem *item);

private slots:
    void editServerConfig();

protected:
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    ServerConfig *m_serverConfig;
    DLabel *      m_nameLabel;
    DLabel *      m_detailsLabel;
    DIconButton * m_rightIconButton;
    DLabel *      m_leftIcon;
    DLabel *      m_rightIcon;
    bool          m_bGroup;
    QString       m_strGroupName;
};

#endif  // SERVERCONFIGITEM_H
