/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:     sunchengxi <sunchengxi@uniontech.com>
 *
 * Maintainer: sunchengxi <sunchengxi@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*******************************************************************************
 1. @类名:    CustomCommandOptDlg
 2. @作者:    ut000125 孙成熙
 3. @日期:    2020-07-31
 4. @说明:    自定义命令操作窗口，进行删除和修改操作
*******************************************************************************/

#ifndef CUSTOMCOMMANDOPTDLG_H
#define CUSTOMCOMMANDOPTDLG_H

#include "termbasedialog.h"
#include "shortcutmanager.h"

#include <DDialog>
#include <DAbstractDialog>
#include <DLineEdit>
#include <DKeySequenceEdit>
#include <DLabel>
#include <DWindowCloseButton>
#include <DPushButton>
#include <DSuggestButton>
#include <DGuiApplicationHelper>

#include <QVBoxLayout>
#include <QAction>
#include <QWidget>

DWIDGET_USE_NAMESPACE

Q_DECLARE_METATYPE(CustomCommandData)

class CustomCommandOptDlg : public DAbstractDialog
{
    Q_OBJECT
public:
    enum CustomCmdOptType {
        CCT_ADD,     // the add type of custom command operation
        CCT_MODIFY,  // the modify type of custom command operation
    };

    explicit CustomCommandOptDlg(CustomCmdOptType type = CCT_ADD,
                                 CustomCommandData *currItemData = nullptr,
                                 QWidget *parent = nullptr);
    ~CustomCommandOptDlg()override;

    void addContent(QWidget *content);
    void addCancelConfirmButtons();

    void setIconPixmap(const QPixmap &iconPixmap);
    void setCancelBtnText(const QString &strCancel);
    void setConfirmBtnText(const QString &strConfirm);
    // 快捷键冲突弹窗
    void showShortcutConflictMsgbox(QString txt);
    void closeRefreshDataConnection();
    bool checkSequence(const QKeySequence &sequence);
    void setModelIndex(QModelIndex mi);

    QDialog::DialogCode getConfirmResult();
    QVBoxLayout *getMainLayout();

    QAction *getCurCustomCmd();
    bool isDelCurCommand();
    CustomCommandData *m_currItemData = nullptr;    //当前自定义列表项数据指针
    QModelIndex modelIndex;                             //模型索引
    DDialog *m_dlgDelete = nullptr;                     //删除确认窗口指针
    unsigned int  m_iTabModifyTime = 0;                 //键盘操作进行修改的时间戳

protected:
    void initUITitle();
    void initTitleConnections();
    //此接口暂时注释保留不删除，2020.12.23
    //void setLogoVisable(bool visible = true);
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
    void slotRefreshData(QString oldCmdName, QString newCmdName);

    void slotNameLineEditingFinished();
    void slotCommandLineEditingFinished();
    void slotShortCutLineEditingFinished(const QKeySequence & sequence);

    void slotCloseButtonClicked();
    void slotThemeTypeChanged(DGuiApplicationHelper::ColorType themeType);

    void slotCancelBtnClicked();
    void slotConfirmBtnClicked();
    void slotShortCutLineEditFinished();
    void slotShortcutConflictDialogFinished();

    void slotSetShortCutLineEditFocus();

private:
    void initUI();
    void initCommandFromClipBoardText();

    CustomCmdOptType m_type;
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
    bool m_bRefreshCheck = false;


    //数字常量
    const int m_iLayoutSizeZero = 0;
    const int m_iLayoutLeftSize = 28;
    const int m_iLayoutRightSize = 30;

    const int m_iFixedWidth = 459;
    const int m_iFixedHeight = 54;

    const int m_iFixedHeightAddSize = 262;
    const int m_iFixedHeightEditSize = 296;

    const int m_iLineEditWidth = 285;

    const int m_iSpaceSizeZero = 0;
    const int m_iSpaceSizeTen = 10;
    const int m_iSpaceSizeEighteen = 18;

    const int m_iSingleShotTime = 30;

};

#endif  // CUSTOMCOMMANDOPTDLG_H
