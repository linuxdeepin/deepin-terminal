#ifndef OPERATIONCONFIRMDLG_H
#define OPERATIONCONFIRMDLG_H

#include <DSuggestButton>
#include <DLabel>
#include <DButtonBox>
#include <DDialog>

#include <QWidget>

DWIDGET_USE_NAMESPACE

class OperationConfirmDlg : public DAbstractDialog
{
    Q_OBJECT
public:
    explicit OperationConfirmDlg(QWidget *parent = nullptr);
    void setOperatTypeName(const QString &strName);
    void setTipInfo(const QString &strInfo);
    void setOKCancelBtnText(const QString &strOk, const QString &strCancel);
    QDialog::DialogCode getConfirmResult();

public slots:
    void slotClickOkButton();
    void slotClickCancelButton();

private:
    DLabel *m_iconLabel = nullptr;
    DLabel *m_operatType = nullptr;
    DLabel *m_tipInfo = nullptr;  // tip  information
    DButtonBoxButton *m_okButton = nullptr;
    DButtonBoxButton *m_cancelButton = nullptr;
    QDialog::DialogCode m_confirmResultCode;
};

#endif  // OPERATIONCONFIRMDLG_H
