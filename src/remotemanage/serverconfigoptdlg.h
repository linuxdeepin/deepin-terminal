// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/*******************************************************************************
 1. @类名:    ServerConfigOptDlg
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    远程管理对话框（弹窗）
             负责添加和修改时给用户提供界面操作的对话框
*******************************************************************************/
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
#include <DPushButton>
#include <DAlertControl>
#include <dfilechooseredit.h>
#include <DWidget>
#include <DScrollArea>

#include <QAction>

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

    /**
     * @brief 服务器配置选项更新数据
     * @author ut000610 daizhengwen
     * @param curServer 当前服务器
     */
    void updataData(ServerConfig *curServer);
    /**
     * @brief 服务器配置选项获取数据
     * @author ut000610 daizhengwen
     * @return
     */
    ServerConfig getData();
    /**
     * @brief 服务器配置选项重置当前服务器
     * @author ut000610 daizhengwen
     * @param curServer 当前服务器
     */
    void resetCurServer(ServerConfig *config);

private slots:
    /**
     * @brief 服务器配置选项增加和保存按钮点击响应函数
     * @author ut000610 daizhengwen
     */
    void slotAddSaveButtonClicked();
    /**
     * @brief 获取FileName
     * @author m000714 戴正文
     */
    void slotFileChooseDialog();
    void handleThemeTypeChanged(DGuiApplicationHelper::ColorType themeType);

private:
    ServerConfigOptType m_type;
    ServerConfig *m_curServer = nullptr;
    DLabel *m_titleLabel = nullptr;
    DLabel *m_iconLabel = nullptr;
    DLineEdit *m_serverName = nullptr;
    DWindowCloseButton *m_closeButton = nullptr;
    DLineEdit *m_address = nullptr;
    DSpinBox *m_port = nullptr;
    DAlertControl *m_port_tip = nullptr;
    DLineEdit *m_userName = nullptr;
    DPasswordEdit *m_password = nullptr;
    DFileChooserEdit *m_privateKey = nullptr;
    DComboBox *m_group = nullptr;
    DLineEdit *m_path = nullptr;
    DLineEdit *m_command = nullptr;
    DComboBox *m_coding = nullptr;
    DComboBox *m_backSapceKey = nullptr;
    DComboBox *m_deleteKey = nullptr;
    bool m_bDelOpt = false;
    QGridLayout *m_pGridLayout = nullptr;

    /**
     * @brief 服务器配置选项初始化UI界面
     * @author ut000610 daizhengwen
     */
    void initUI();
    /**
     * @brief 服务器配置选项初始化数据
     * @author ut000610 daizhengwen
     */
    void initData();
    /**
     * @brief 服务器配置选项获取文本的编码
     * @author ut000610 daizhengwen
     * @return
     */
    QList<QString> getTextCodec();
    /**
     * @brief 服务器配置选项获取退格键
     * @author ut000610 daizhengwen
     * @return
     */
    QList<QString> getBackSpaceKey();
    /**
     * @brief 服务器配置选项获取删除键
     * @author ut000610 daizhengwen
     * @return
     */
    QList<QString> getDeleteKey();

    QString m_currentServerName;

    /**
     * @brief 设置label格式
     * @author m000714 戴正文
     */
    inline void setLabelStyle(DLabel *);

};

#endif  // SERVERCONFIGOPTDLG_H
