// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CUSTOMTHEMESETTINGDIALOG_H
#define CUSTOMTHEMESETTINGDIALOG_H

#include "themepreviewarea.h"

#include <DDialog>
#include <DAbstractDialog>
#include <DLineEdit>
#include <DKeySequenceEdit>
#include <DLabel>
#include <DWindowCloseButton>
#include <DPushButton>
#include <DSuggestButton>
#include <DColorDialog>
#include <DRadioButton>
#include <DGroupBox>

#include <QButtonGroup>
#include <QVBoxLayout>
#include <QAction>
#include <QWidget>
#include <QRadioButton>
#include <QButtonGroup>
#include <QKeyEvent>
#include <QVariant>

DWIDGET_USE_NAMESPACE

/*******************************************************************************
 1. @类名:    TitleStyleRadioButton
 2. @作者:    ut000125 sunchengxi
 3. @日期:    2020-12-01
 4. @说明:    主题风格单选按钮类
*******************************************************************************/
class TitleStyleRadioButton: public DRadioButton
{
    Q_OBJECT
public:
    explicit TitleStyleRadioButton(const QString &text, QWidget *parent = nullptr);
protected:
    /**
     * @brief 设置背景色
     * @author ut000125 sunchengxi
     * @param event 鼠标按下事件
     */
    void mousePressEvent(QMouseEvent *event) override;
    /**
     * @brief 按键按下事件
     * @author ut000125 sunchengxi
     * @param event 按键按下事件
     */
    void keyPressEvent(QKeyEvent *event) override;
public:
    //鼠标操作标志位
    bool m_mouseClick = false;
};

/*******************************************************************************
 1. @类名:    ColorPushButton
 2. @作者:    ut000125 sunchengxi
 3. @日期:    2020-12-01
 4. @说明:    带背景色按钮类
*******************************************************************************/
class ColorPushButton: public DPushButton
{
    Q_OBJECT
public:
    /**
     * @brief 构造函数初始化
     * @author ut000125 sunchengxi
     * @param parent
     */
    explicit ColorPushButton(QWidget *parent = nullptr);
    /**
     * @brief 设置背景色
     * @author ut000125 sunchengxi
     * @param color 颜色
     */
    void setBackGroundColor(const QColor &color);
    /**
     * @brief 获取背景色
     * @author ut000125 sunchengxi
     * @return
     */
    QColor getBackGroundColor();

signals:
    //清理按钮焦点信号
    void clearFocussSignal();

protected:
    /**
     * @brief 处理重绘事件
     * @anchor ut000125 sunchengxi
     * @param event 重绘事件
     */
    void paintEvent(QPaintEvent *event) override;
    /**
     * @brief 焦点进入事件
     * @author ut000125 sunchengxi
     * @param event 事件
     */
    void focusInEvent(QFocusEvent *event) override;
    //焦点离开事件
    /*******************************************************************************
     1. @函数:    focusOutEvent
     2. @作者:    ut000125 sunchengxi
     3. @日期:    2020-12-01
     4. @说明:    焦点离开事件
    *******************************************************************************/
    /**
     * @brief 焦点离开事件
     * @author ut000125 sunchengxi
     * @param event 事件
     */
    void focusOutEvent(QFocusEvent *event) override;
    /**
     * @brief 按键按下事件
     * @author ut000125 sunchengxi
     * @param event 事件
     */
    void keyPressEvent(QKeyEvent *event) override;
    /**
     * @brief 鼠标按下事件
     * @author ut000125 sunchengxi
     * @param event 事件
     */
    void mousePressEvent(QMouseEvent *event) override;
public:
    //背景色
    QColor m_color;
    //焦点是否在 用于背景和边框
    bool m_isFocus = false;

};

/*******************************************************************************
 1. @类名:    CustomThemeSettingDialog
 2. @作者:    ut000125 sunchengxi
 3. @日期:    2020-12-01
 4. @说明:    自定义主题设置对话框类
*******************************************************************************/
class CustomThemeSettingDialog : public DAbstractDialog
{
    Q_OBJECT
public:
    explicit CustomThemeSettingDialog(QWidget *parent = nullptr);
    /**
     * @brief 重置单选按钮的tab焦点状态，保证每次打开时，第一个单选按钮是tab键盘控制的选中按钮
     * @author ut000125 sunchengxi
     */
    void resetFocusState();

protected:
    /**
     * @brief 标题栏初始化
     * @author ut000125 sunchengxi
     */
    void initUITitle();
    /**
     * @brief 工作区初始化
     * @author ut000125 sunchengxi
     */
    void initUI();
    /**
     * @brief 标题栏初始化信号槽
     * @author ut000125 sunchengxi
     */
    void initTitleConnections();
    /**
     * @brief 增加确认取消按钮
     * @author ut000125 sunchengxi
     */
    void addCancelConfirmButtons();
    /**
     * @brief 加载主题配置
     * @author ut000125 sunchengxi
     */
    void loadConfiguration();
    /**
     * @brief 键盘事件
     * @author ut000125 sunchengxi
     * @param 事件
     */
    void keyPressEvent(QKeyEvent *event) override;
    /**
     * @brief 显示事件
     * @author ut000125 sunchengxi
     * @param 事件
     */
    void showEvent(QShowEvent *event) override;

public slots:
    /**
     * @brief 选择配色槽
     * @author ut000125 sunchengxi
     */
    void onSelectColor();
    /**
     * @brief 清理按钮焦点槽
     * @author ut000125 sunchengxi
     */
    void clearFocussSlot();

private:
    //标题栏
    QWidget *m_titleBar = nullptr;
    //用于标题布局占位的label
    DLabel  *m_logoIcon = nullptr;
    //标题的文本label
    DLabel  *m_titleText = nullptr;
    //标题栏里的关闭按钮
    DWindowCloseButton *m_closeButton = nullptr;
    //自定义配色框的工作区
    QWidget *m_content = nullptr;
    //自定义配色框的工作区布局
    QVBoxLayout *m_contentLayout = nullptr;
    //自定义配色框整体布局
    QVBoxLayout *m_mainLayout = nullptr;
    //取消按钮
    DPushButton *m_cancelBtn = nullptr;
    //确认按钮
    DSuggestButton *m_confirmBtn = nullptr;
    //预览区域
    ThemePreviewArea *m_themePreviewArea = nullptr;
    //深色主题风格单选按钮
    TitleStyleRadioButton *m_darkRadioButton = nullptr;
    //浅色主题风格单选按钮
    TitleStyleRadioButton *m_lightRadioButton = nullptr;
    //单选按钮组
    QButtonGroup  *m_titleStyleButtonGroup = nullptr;
    //前景色标签
    DLabel *m_foregroundColorLabel = nullptr;
    //前景色按钮
    ColorPushButton *m_foregroundButton = nullptr;
    //背景色标签
    DLabel *m_backgroundColorLabel = nullptr;
    //背景色按钮
    ColorPushButton *m_backgroundButton = nullptr;
    //提示符PS1标签
    DLabel *m_ps1ColorLabel = nullptr;
    //提示符PS1按钮
    ColorPushButton *m_ps1Button = nullptr;
    //提示符PS2标签
    DLabel *m_ps2ColorLabel = nullptr;
    //提示符PS2按钮
    ColorPushButton *m_ps2Button = nullptr;

};

#endif // CUSTOMTHEMESETTINGDIALOG_H
