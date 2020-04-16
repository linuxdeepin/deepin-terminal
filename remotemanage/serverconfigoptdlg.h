#ifndef SERVERCONFIGOPTDLG_H
#define SERVERCONFIGOPTDLG_H

#include "serverconfigmanager.h"

#include <DAbstractDialog>
#include <DApplicationHelper>
#include <DButtonBox>
#include <DComboBox>
#include <DCommandLinkButton>
#include <DDialogCloseButton>
#include <DWindowCloseButton>
#include <DIconButton>
#include <DLabel>
#include <DLineEdit>
#include <DPasswordEdit>
#include <DSpinBox>
#include <dfilechooseredit.h>

#include <QAction>
#include <QWidget>

DWIDGET_USE_NAMESPACE

class TermCommandLinkButton;

class ServerConfigOptDlg : public DAbstractDialog
{
    Q_OBJECT
public:
    enum ServerConfigOptType {
        SCT_ADD,     // the add type of server config operation
        SCT_MODIFY,  // the modify type of server config operation
    };

    explicit ServerConfigOptDlg(ServerConfigOptType type = SCT_ADD, ServerConfig *curServer = nullptr,
                                QWidget *parent = nullptr);
    ~ServerConfigOptDlg();

    ServerConfig *getCurServer()
    {
        return m_curServer;
    }

    bool isDelServer()
    {
        return m_bDelOpt;
    }

    void setDelServer(bool isDel)
    {
        m_bDelOpt = isDel;
    }

    const QString &getServerName()
    {
        return m_currentServerName;
    }

private slots:
    void slotClose();
    void slotAddSaveButtonClicked();

private:
    ServerConfigOptType m_type;
    ServerConfig *m_curServer = nullptr;
    DLabel *m_titleLabel = nullptr;
    DLabel *m_iconLabel = nullptr;
    DWindowCloseButton *m_closeButton = nullptr;
    DLineEdit *m_serverName = nullptr;
    DLineEdit *m_address = nullptr;
    DSpinBox *m_port = nullptr;
    DLineEdit *m_userName = nullptr;
    DPasswordEdit *m_password = nullptr;
    DFileChooserEdit *m_privateKey = nullptr;
    DLineEdit *m_group = nullptr;
    DLineEdit *m_path = nullptr;
    DLineEdit *m_command = nullptr;
    DComboBox *m_coding = nullptr;
    DComboBox *m_backSapceKey = nullptr;
    DComboBox *m_deleteKey = nullptr;
    DCommandLinkButton *m_advancedOptions = nullptr;
    TermCommandLinkButton *m_delServer = nullptr;
    bool m_bDelOpt = false;

    void initUI();
    void initData();
    QList<QString> getTextCodec();
    QList<QString> getBackSpaceKey();
    QList<QString> getDeleteKey();

    QString m_currentServerName = "";

    inline void setLabelStyle(DLabel *);

};

#endif  // SERVERCONFIGOPTDLG_H
