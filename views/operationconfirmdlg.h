#ifndef OPERATIONCONFIRMDLG_H
#define OPERATIONCONFIRMDLG_H

#include <DSuggestButton>
#include <DLabel>
#include <DButtonBox>
#include <DDialog>
#include <DWindowCloseButton>
#include <DPushButton>
#include <DWarningButton>

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE

class OperationConfirmDlg : public DAbstractDialog
{
    Q_OBJECT
public:
    explicit OperationConfirmDlg(QWidget *parent = nullptr);

    void addContent(QWidget *content);
    void setIconPixmap(const QPixmap &iconPixmap);
    void setTitle(const QString &title);

    void setOperatTypeName(const QString &strName);
    void setTipInfo(const QString &strInfo);
    void setOKCancelBtnText(const QString &strConfirm, const QString &strCancel);
    QDialog::DialogCode getConfirmResult();

    void setDialogFrameSize(int width, int height);

protected:
    QLayout *getContentLayout();
    void closeEvent(QCloseEvent *event) override;

private:
    void initUI();
    void initContentLayout();
    void initConnections();

    DLabel  *m_logoIcon = nullptr;
    QWidget *m_titleBar = nullptr;
    DLabel  *m_titleText = nullptr;

    QWidget *m_content = nullptr;
    QVBoxLayout *m_contentLayout = nullptr;
    QVBoxLayout *m_mainLayout = nullptr;
    QHBoxLayout *m_actionLayout = nullptr;

    DLabel *m_operateTypeName = nullptr;
    DLabel *m_tipInfo = nullptr;
    DPushButton *m_cancelBtn = nullptr;
    DWarningButton *m_confirmBtn = nullptr;
    DWindowCloseButton *m_closeButton = nullptr;
    QDialog::DialogCode m_confirmResultCode;

signals:
    void closed();
};

#endif  // OPERATIONCONFIRMDLG_H
