/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:  daizhengwen<daizhengwen@uniontech.com>
 *
 * Maintainer: daizhengwen<daizhengwen@uniontech.com>
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

/**
 * 远程管理对话框（弹窗）
 * 负责添加和修改时给用户提供界面操作的对话框
 */
#ifndef SERVERCONFIGOPTDLG_H
#define SERVERCONFIGOPTDLG_H

#include "serverconfigmanager.h"

#include <DAbstractDialog>
#include <DApplicationHelper>
#include <DButtonBox>
#include <DComboBox>
#include <DCommandLinkButton>
#include <DDialogCloseButton>
#include <DWindowCloseButton>
#include <DIconButton>
#include <DLabel>
#include <DSpinBox>
#include <DLineEdit>
#include <DPasswordEdit>
#include <DSuggestButton>
#include <DPushButton>
#include <dfilechooseredit.h>

#include <QAction>
#include <QWidget>

DWIDGET_USE_NAMESPACE

// 默认端口
#define DEFAULTPORT 22
// 控件默认高度
#define DEFAULTHEIGHT 290

class TermCommandLinkButton;

class ServerConfigOptDlg : public DAbstractDialog
{
    Q_OBJECT
public:
    enum ServerConfigOptType {
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

    const QString &getServerName()
    {
        return m_currentServerName;
    }

    void updataData(ServerConfig *curServer);
    ServerConfig getData();
    void resetCurServer(ServerConfig curServer);

private slots:
    void slotAddSaveButtonClicked();
    void slotFileChooseDialog();

private:
    ServerConfigOptType m_type;
    ServerConfig *m_curServer = nullptr;
    DLabel *m_titleLabel = nullptr;
    DLabel *m_iconLabel = nullptr;
    DWindowCloseButton *m_closeButton = nullptr;
    DLineEdit *m_serverName = nullptr;
    DLineEdit *m_address = nullptr;
    DSpinBox *m_port = nullptr;
    DLineEdit *m_userName = nullptr;
    DPasswordEdit *m_password = nullptr;
//    DFileChooserEdit *m_privateKey = nullptr;
    DLineEdit *m_privateKey = nullptr;
    DSuggestButton *m_fileDialog = nullptr;
    DLineEdit *m_group = nullptr;
    DLineEdit *m_path = nullptr;
    DLineEdit *m_command = nullptr;
    DComboBox *m_coding = nullptr;
    DComboBox *m_backSapceKey = nullptr;
    DComboBox *m_deleteKey = nullptr;
//    DCommandLinkButton *m_advancedOptions = nullptr;
    DPushButton *m_advancedOptions = nullptr;
    TermCommandLinkButton *m_delServer = nullptr;
    bool m_bDelOpt = false;

    void initUI();
    void initData();
    QList<QString> getTextCodec();
    QList<QString> getBackSpaceKey();
    QList<QString> getDeleteKey();

    QString m_currentServerName;

    inline void setLabelStyle(DLabel *);

};

#endif  // SERVERCONFIGOPTDLG_H
