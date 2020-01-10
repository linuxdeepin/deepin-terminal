#ifndef SERVERCONFIGOPTDLG_H
#define SERVERCONFIGOPTDLG_H

#include <QWidget>
#include <DAbstractDialog>
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
#include <QAction>
#include <DDialogCloseButton>

DWIDGET_USE_NAMESPACE
class ServerConfigOptDlg : public DAbstractDialog
{
    Q_OBJECT
public:
    enum ServerConfigOptType {
        SCT_ADD,// the add type of server config operation
        SCT_MODIFY, // the modify type of server config operation
    };
    explicit ServerConfigOptDlg(ServerConfigOptType type = SCT_ADD, QAction *curAction = nullptr, QWidget *parent = nullptr);
    ~ServerConfigOptDlg();
    QAction &getCurAction() {return m_curAction;}
signals:

private slots:
    void slotClose();
private:
    ServerConfigOptType m_type;
    QAction *m_action;
    QAction m_curAction;
    DLabel *m_titleLabel;
    DDialogCloseButton *m_closeButton;
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

#endif // SERVERCONFIGOPTDLG_H
