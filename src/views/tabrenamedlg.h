// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TABRENAMEDLG_H
#define TABRENAMEDLG_H

#include "tabrenamewidget.h"

#include <DSuggestButton>
#include <DLabel>
#include <DButtonBox>
#include <DDialog>
#include <DWindowCloseButton>
#include <DPushButton>
#include <DToolButton>

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE

class TabRenameWidget;
class TabRenameDlg : public DAbstractDialog
{
    Q_OBJECT
public:
    /**
     * @brief 标签重命名构造函数
     * @author ut000442 赵公强
     * @param parent
     */
    explicit TabRenameDlg(QWidget *parent = nullptr);

    /**
     * @brief 初始化主窗口的ui
     * @author ut000442 赵公强
     */
    void initUi();
    /**
     * @brief 初始化链接
     * @author ut000442 赵公强
     */
    void initConnections();
    /**
     * @brief 初始化窗口包含的控件
     * @author ut000442 赵公强
     */
    void initContentWidget();
    /**
     * @brief 初始化标题
     * @author ut000442 赵公强
     */
    void initTitleLabel();
    /**
     * @brief 初始化窗口中第二，三行，参数 isRemote表示是否为远程
     * @author ut000442 赵公强
     * @param isRemote 是否为远程
     */
    void initRenameWidget(bool isRemote);
    /**
     * @brief 初始化按钮控件
     * @author ut000442 赵公强
     */
    void initButtonWidget();

    /**
     * @brief 向主窗口中添加控件
     * @author ut000442 赵公强
     * @param content
     */
    void addContent(QWidget *content);
    /**
     * @brief 获取主窗口最外层布局接口
     * @author ut000442 赵公强
     * @return
     */
    QLayout *getContentLayout();

    /**
     * @brief 设置lineedit显示的文字
     * @author ut000610 戴正文
     * @param tabTitleFormat
     * @param remoteTabTitleFormat
     */
    void setText(const QString &tabTitleFormat, const QString &remoteTabTitleFormat);
    /**
     * @brief 窗口普通输入条内容设置接口
     * @author ut000442 赵公强
     * @param text 文本
     */
    void setNormalLineEditText(const QString &text);
    /**
     * @brief 窗口远程输入条内容设置接口
     * @author ut000442 赵公强
     * @param text 文本
     */
    void setRemoteLineEditText(const QString &text);

    /**
     * @brief 获取普通重命名控件接口
     * @author ut000442 赵公强
     * @return
     */
    QLineEdit *getTabTitleEdit() const;
    /**
     * @brief 获取远程重命名控件接口
     * @author ut000442 赵公强
     * @return
     */
    QLineEdit *getRemoteTabTitleEdit() const;

    /**
     * @brief 将焦点设置在输入框内
     * @author ut000610 戴正文
     * @param isRemote 是否远程
     */
    void setFocusOnEdit(bool isRemote);

Q_SIGNALS:
    void tabTitleFormatRename(const QString &tabTitleFormat, const QString &remoteTabTitleFormat);

private:
    QWidget *m_titleBar = nullptr;
    QWidget *m_content = nullptr;
    QWidget *m_normalWidget = nullptr;
    QWidget *m_remoteWidget = nullptr;
    QWidget *m_buttonWidget = nullptr;

    // 标签标题格式
    TabRenameWidget *m_tabTitleEdit = nullptr;
    // 远程标签标题格式
    TabRenameWidget *m_remoteTabTitleEdit = nullptr;

    QVBoxLayout *m_contentLayout = nullptr;
    QVBoxLayout *m_mainLayout = nullptr;

    DLabel  *m_logoIcon = nullptr;
    DLabel  *m_titleText = nullptr;

    DLabel *m_titlelabel = nullptr;
    DPushButton *m_cancelButton = nullptr;
    DSuggestButton *m_confirmButton = nullptr;

    DWindowCloseButton *m_closeButton = nullptr;

};

#endif // TABRENAMEDLG_H
