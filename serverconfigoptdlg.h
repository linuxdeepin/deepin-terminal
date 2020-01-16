#ifndef SERVERCONFIGOPTDLG_H
#define SERVERCONFIGOPTDLG_H

#include <DAbstractDialog>
#include <DApplicationHelper>
#include <DButtonBox>
#include <DComboBox>
#include <DCommandLinkButton>
#include <DDialogCloseButton>
#include <DIconButton>
#include <DLabel>
#include <DLineEdit>
#include <DPasswordEdit>
#include <DSpinBox>
#include <QAction>
#include <QWidget>
#include <dfilechooseredit.h>
#include <serverconfigmanager.h>

DWIDGET_USE_NAMESPACE
class ServerConfigOptDlg : public DAbstractDialog
{
    Q_OBJECT
public:
    enum ServerConfigOptType
    {
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
signals:

private slots:
    void initUI();
    void initData();
    void slotClose();
    void slotAddSaveButtonClicked();

private:
    ServerConfigOptType m_type;
    ServerConfig *      m_curServer;
    DLabel *            m_titleLabel;
    DDialogCloseButton *m_closeButton;
    DLineEdit *         m_serverName;
    DLineEdit *         m_address;
    DSpinBox *          m_port;
    DLineEdit *         m_userName;
    DPasswordEdit *     m_password;
    DFileChooserEdit *  m_privateKey;
    DLineEdit *         m_group;
    DLineEdit *         m_path;
    DLineEdit *         m_command;
    DComboBox *         m_coding;
    DComboBox *         m_backSapceKey;
    DComboBox *         m_deleteKey;
    DCommandLinkButton *m_advancedOptions;
    DCommandLinkButton *m_delServer;
    bool                m_bDelOpt = false;
};

#endif  // SERVERCONFIGOPTDLG_H
