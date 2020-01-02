#ifndef CUSTOMCOMMANDOPTDLG_H
#define CUSTOMCOMMANDOPTDLG_H

#include <QWidget>
#include <DDialog>
#include <QAction>
#include <DAbstractDialog>
#include <DLineEdit>
#include <DKeySequenceEdit>
DWIDGET_USE_NAMESPACE
class CustomCommandOptDlg : public DAbstractDialog
{
    Q_OBJECT
public:
    enum CustomCmdOptType {
        CCT_ADD,// the add type of custom command operation
        CCT_MODIFY, // the modify type of custom command operation
    };

    explicit CustomCommandOptDlg(CustomCmdOptType type = CCT_ADD, QAction *curAction = nullptr, QWidget *parent = nullptr);
    ~CustomCommandOptDlg();
    QAction &getCurCustomCmd() {return m_newAction;}
    bool isDelCurCommand() {return m_bDelOpt;}
private slots:
    void slotAddSaveButtonClicked();
    void slotDelCurCustomCommand();
private:
    CustomCmdOptType m_type;
    QAction *m_action;
    QAction m_newAction;
    DLineEdit *m_nameLineEdit;
    DLineEdit *m_commandLineEdit;
    DKeySequenceEdit *m_shortCutsLineEdit;
    bool m_bDelOpt;
};

#endif // CUSTOMCOMMANDOPTDLG_H
