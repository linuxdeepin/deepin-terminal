#ifndef SERVERCONFIGOPTWIDGET_H
#define SERVERCONFIGOPTWIDGET_H

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

#include <QWidget>

DWIDGET_USE_NAMESPACE

class ServerConfigOptWidget : public QWidget
{
    Q_OBJECT
public:
    enum ServerConfigOptType
    {
        SCT_ADD,     // the add type of server config operation
        SCT_MODIFY,  // the modify type of server config operation
    };
    explicit ServerConfigOptWidget(QWidget *parent = nullptr);

private slots:
    void slotClose();

private:
    DLabel *m_titleLabel = nullptr;
    DIconButton *m_closeButton = nullptr;
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
    DCommandLinkButton *m_delServer = nullptr;
};

#endif  // SERVERCONFIGOPTWIDGET_H
