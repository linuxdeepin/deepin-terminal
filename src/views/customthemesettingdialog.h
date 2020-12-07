/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
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
    //鼠标按下事件
    void mousePressEvent(QMouseEvent *event) override;
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
    explicit ColorPushButton(QWidget *parent = nullptr);
    //设置背景色
    void setBackGroundColor(const QColor &color);
    //获取背景色
    QColor getBackGroundColor();

signals:
    //清理按钮焦点信号
    void clearFocussSignal();

protected:
    //处理重绘事件
    void paintEvent(QPaintEvent *event) override;
    //焦点进入事件
    void focusInEvent(QFocusEvent *event) override;
    //焦点离开事件
    void focusOutEvent(QFocusEvent *event) override;
    //按键按下事件
    void keyPressEvent(QKeyEvent *event) override;
    //鼠标按下事件
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

protected:
    //标题栏初始化
    void initUITitle();
    //工作区初始化
    void initUI();
    //标题栏初始化信号槽
    void initTitleConnections();
    //增加确认取消按钮
    void addCancelConfirmButtons();
    //加载主题配置
    void loadConfiguration();
    //键盘事件
    void keyPressEvent(QKeyEvent *event) override;
    //显示事件
    void showEvent(QShowEvent *event) override;

public slots:
    //选择配色槽
    void onSelectColor();
    //清理按钮焦点槽
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
