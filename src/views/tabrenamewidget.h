// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TABRENAMEWIDGET_H
#define TABRENAMEWIDGET_H

#include <DLineEdit>
#include <DPushButton>
#include <DMenu>

#include <QLabel>
#include <QWidget>
#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE

class TabRenameWidget : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief 标签重命名控件构造函数，由输入条与一个内置菜单的按钮组成
     * @author ut000442 赵公强
     * @param isRemote 是否为远程
     * @param isSetting 是否在设置中调用
     * @param parent
     */
    explicit TabRenameWidget(bool isRemote, bool isSetting = false, QWidget *parent = nullptr);

    /**
     * @brief 初始化标签重命名控件的ui
     * @author ut000442 赵公强
     */
    void initUi();
    /**
     * @brief 初始化按钮内置的菜单
     * @author ut000442 赵公强
     */
    void initChoseMenu();
    /**
     * @brief 初始化远程标签按钮内置的菜单
     * @author ut000442 赵公强
     */
    void initRemoteChoseMenu();
    /**
     * @brief 初始化普通标签按钮内置的菜单
     * @author ut000442 赵公强
     */
    void initNormalChoseMenu();
    /**
     * @brief 初始化链接
     * @author ut000442 赵公强
     */
    void initConnections();

    /**
     * @brief 初始化小标题
     * @author ut000442 赵公强
     */
    void initLabel();
    void setLineEditText();

    /**
     * @brief 获取输入条接口
     * @author ut000442 赵公强
     * @return
     */
    DLineEdit *getInputedit() const;

private:
    bool m_isRemote = false;
    bool m_isSetting = false;

    QLabel *m_Label = nullptr;
    QHBoxLayout *m_layout = nullptr;

    DPushButton *m_choseButton = nullptr;
    DMenu *m_choseMenu = nullptr;
    DLineEdit *m_inputedit = nullptr;
};

#endif // TABRENAMEWIDGET_H
