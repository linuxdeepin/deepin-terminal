#ifndef OPERATIONCONFIRMDLG_H
#define OPERATIONCONFIRMDLG_H

#include <QWidget>
#include <DSuggestButton>
#include <DLabel>
#include <DButtonBox>
#include <DDialog>
DWIDGET_USE_NAMESPACE
class OperationConfirmDlg : public DAbstractDialog
{
    Q_OBJECT
public:
    explicit OperationConfirmDlg(QWidget *parent = nullptr);
    void setOperatTypeName(const QString &strName);
    void setTipInfo(const QString &strInfo);
    QDialog::DialogCode getConfirmResult();
signals:

public slots:
    void slotClickOkButton();
    void slotClickCancelButton();
private:
    DLabel *m_iconLabel;
    DLabel *m_operatType;
    DLabel *m_tipInfo;//tip  information
    DButtonBoxButton *m_okButton;
    DButtonBoxButton *m_cancelButton;
    QDialog::DialogCode m_confirmResultCode;
};

#endif // OPERATIONCONFIRMDLG_H
