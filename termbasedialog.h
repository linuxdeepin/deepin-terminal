#ifndef TERMBASEDIALOG_H
#define TERMBASEDIALOG_H

#include <DAbstractDialog>
#include <DLabel>
#include <DWindowCloseButton>
#include <DPushButton>
#include <DSuggestButton>

#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

class TermBaseDialog : public DAbstractDialog
{
    Q_OBJECT
public:
    explicit TermBaseDialog(QWidget *parent = nullptr);

    void addContent(QWidget* content);
    void addCancelConfirmButtons();

    void setIconPixmap(const QPixmap &iconPixmap);
    void setCancelBtnText(const QString &strCancel);
    void setConfirmBtnText(const QString &strConfirm);

    QDialog::DialogCode getConfirmResult();
    QVBoxLayout *getMainLayout();

protected:
    void initUI();
    void initConnections();
    void setLogoVisable(bool visible=true);
    void setTitle(const QString& title);
    QLayout* getContentLayout();

    //Overrides
    void closeEvent(QCloseEvent *event) override;
signals:
    void closed();
    void confirmBtnClicked();

private:
    QWidget *m_titleBar = nullptr;
    DLabel  *m_logoIcon = nullptr;
    DLabel  *m_titleText = nullptr;
    DWindowCloseButton* m_closeButton = nullptr;

    QWidget *m_content = nullptr;
    QVBoxLayout *m_contentLayout = nullptr;
    QVBoxLayout *m_mainLayout = nullptr;

    DPushButton *m_cancelBtn = nullptr;
    DSuggestButton *m_confirmBtn = nullptr;

    QDialog::DialogCode m_confirmResultCode;
};

#endif // TERMBASEDIALOG_H
