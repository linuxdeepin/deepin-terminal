#ifndef CUSTOMCOMMANDOPTDLG_H
#define CUSTOMCOMMANDOPTDLG_H

#include "termbasedialog.h"
#include "customcommanditemmodel.h"

#include <DDialog>
#include <DPasswordEdit>
#include <DKeySequenceEdit>

#include <QAction>
#include <QWidget>

DWIDGET_USE_NAMESPACE

class CustomCommandOptDlg : public TermBaseDialog
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

    QAction *getCurCustomCmd();
    bool isDelCurCommand();

private slots:
    void slotAddSaveButtonClicked();
    void slotDelCurCustomCommand();

private:
    void initUI();
    void initCommandFromClipBoardText();

    CustomCmdOptType m_type;
    CustomCommandItemData *m_currItemData = nullptr;
    QAction *m_newAction = nullptr;
    DPasswordEdit *m_nameLineEdit = nullptr;
    DPasswordEdit *m_commandLineEdit = nullptr;
    DKeySequenceEdit *m_shortCutLineEdit = nullptr;
    bool m_bDelOpt;
};

#endif  // CUSTOMCOMMANDOPTDLG_H
