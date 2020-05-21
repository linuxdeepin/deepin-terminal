#ifndef CUSTOMCOMMANDOPTDLG_H
#define CUSTOMCOMMANDOPTDLG_H

#include "termbasedialog.h"
#include "customcommanditemmodel.h"

#include <DDialog>
#include <DAbstractDialog>
#include <DLineEdit>
#include <DKeySequenceEdit>
#include <DLabel>
#include <DWindowCloseButton>
#include <DPushButton>
#include <DSuggestButton>

#include <QVBoxLayout>
#include <QAction>
#include <QWidget>

DWIDGET_USE_NAMESPACE

class CustomCommandOptDlg : public DAbstractDialog
{
    Q_OBJECT
public:
    enum CustomCmdOptType {
        CCT_ADD,     // the add type of custom command operation
        CCT_MODIFY,  // the modify type of custom command operation
    };

    explicit CustomCommandOptDlg(CustomCmdOptType type = CCT_ADD,
                                 CustomCommandItemData *currItemData = nullptr,
                                 QWidget *parent = nullptr);
    ~CustomCommandOptDlg();

    void addContent(QWidget *content);
    void addCancelConfirmButtons();

    void setIconPixmap(const QPixmap &iconPixmap);
    void setCancelBtnText(const QString &strCancel);
    void setConfirmBtnText(const QString &strConfirm);
    // 快捷键冲突弹窗
    void showShortcutConflictMsgbox(QString txt);

    QDialog::DialogCode getConfirmResult();
    QVBoxLayout *getMainLayout();

    QAction *getCurCustomCmd();
    bool isDelCurCommand();
    bool m_bNeedDel = false;
protected:
    void initUITitle();
    void initTitleConnections();
    void setLogoVisable(bool visible = true);
    void setTitle(const QString &title);
    QLayout *getContentLayout();

    //Overrides
    void closeEvent(QCloseEvent *event) override;
signals:
    void closed();
    void confirmBtnClicked();

private slots:
    void slotAddSaveButtonClicked();
    void slotDelCurCustomCommand();

private:
    void initUI();
    void initCommandFromClipBoardText();

    CustomCmdOptType m_type;
    CustomCommandItemData *m_currItemData = nullptr;
    QAction *m_newAction = nullptr;
    DLineEdit *m_nameLineEdit = nullptr;
    DLineEdit *m_commandLineEdit = nullptr;
    DKeySequenceEdit *m_shortCutLineEdit = nullptr;
    QString m_lastCmdShortcut;
    bool m_bDelOpt;

    QWidget *m_titleBar = nullptr;
    DLabel  *m_logoIcon = nullptr;
    DLabel  *m_titleText = nullptr;
    DWindowCloseButton *m_closeButton = nullptr;

    QWidget *m_content = nullptr;
    QVBoxLayout *m_contentLayout = nullptr;
    QVBoxLayout *m_mainLayout = nullptr;

    DPushButton *m_cancelBtn = nullptr;
    DSuggestButton *m_confirmBtn = nullptr;

    QDialog::DialogCode m_confirmResultCode;
    // 快捷键冲突弹窗
    DDialog *m_shortcutConflictDialog = nullptr;
};

#endif  // CUSTOMCOMMANDOPTDLG_H
