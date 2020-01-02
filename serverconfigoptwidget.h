#ifndef SERVERCONFIGOPTWIDGET_H
#define SERVERCONFIGOPTWIDGET_H

#include <QWidget>
#include <DLabel>
#include <DComboBox>
#include <DCommandLinkButton>
#include <DButtonBox>
#include <DApplicationHelper>
#include <DIconButton>
#include <DLineEdit>
#include <DSpinBox>
#include <DPasswordEdit>
#include <dfilechooseredit.h>

DWIDGET_USE_NAMESPACE
class ServerConfigOptWidget : public QWidget
{
    Q_OBJECT
public:
    enum ServerConfigOptType {
        SCT_ADD,// the add type of server config operation
        SCT_MODIFY, // the modify type of server config operation
    };
    explicit ServerConfigOptWidget(QWidget *parent = nullptr);

signals:

public slots:
private slots:
    void slotClose();
private:
    DLabel *m_titleLabel;
    DIconButton *m_closeButton;
    DLineEdit *m_serverName;
    DLineEdit *m_address;
    DSpinBox *m_port;
    DLineEdit *m_userName;
    DPasswordEdit *m_password;
    DFileChooserEdit *m_privateKey;
    DLineEdit *m_group;
    DLineEdit *m_path;
    DLineEdit *m_command;
    DComboBox *m_coding;
    DComboBox *m_backSapceKey;
    DComboBox *m_deleteKey;
    DCommandLinkButton *m_advancedOptions;
    DCommandLinkButton *m_delServer;




};

#endif // SERVERCONFIGOPTWIDGET_H
