// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/*******************************************************************************
 1. @类名:    ItemWidget
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    项控件
             可以通过类型选择选择项控件的展示内容
             1）分组项
             2）普通项
*******************************************************************************/
#ifndef ITEMWIDGET_H
#define ITEMWIDGET_H
// custom
#include "focusframe.h"

// dtk
//#include <DFrame>
#include <DCheckBox>
#include <DIconButton>
#include <DLabel>
#include <DGuiApplicationHelper>
#include <DFontSizeManager>
#include <DPalette>
#include <DApplicationHelper>
// qt
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QFocusEvent>
#include <QKeyEvent>

DWIDGET_USE_NAMESPACE
#define COORDINATE_ERROR_Y  5

// 功能键类型
enum ItemFuncType {
    // 项按钮（编辑按钮）
    ItemFuncType_Item = 0,
    // 项标签
    ItemFuncType_ItemLabel = 1,
    // 分组按钮（用于远程管理显示分组）
    ItemFuncType_Group = 2,
    // 分组标签
    ItemFuncType_GroupLabel = 3,
    // 未分组可勾选项
    ItemFuncType_UngroupedItem = 4
};

// 文字颜色
enum ItemTextColor {
    // Text      // 第一行颜色
    ItemTextColor_Text = 0,
    // TextTips  // 第二行颜色
    ItemTextColor_TextTips
};

class IconButton;
// 远程管理项的通用view类
class ItemWidget : public FocusFrame
{
    Q_OBJECT
public:
    ItemWidget(ItemFuncType itemType, QWidget *parent = nullptr);
    ~ItemWidget();

    /**
     * @brief 根据名称设置图标(远程组图标 dt_server_group 远程服务器图标 dt_server 自定义图标 dt_command)
     * @author ut000610 戴正文
     * @param 图标名称
     */
    void setIcon(const QString &icon);
    /**
     * @brief 根据类型设置功能图标 (编辑 dt_edit 显示分组 dt_arrow_right)
     * @author ut000610 戴正文
     * @param 图标类型
     */
    void setFuncIcon(ItemFuncType iconType);
    /**
     * @brief 获取功能键类型
     * @author ut000610 戴正文
     * @return
     */
    ItemFuncType getFuncType();

    /**
     * @brief 设置文字内容
     * @author ut000610 戴正文
     * @param firstline 第一行信息
     * @param secondline 第二行信息
     */
    void setText(const QString &firstline, const QString &secondline = "");
    /**
     * @brief 获取第一行信息
     * @author ut000438 王亮
     * @return
     */
    const QString getFirstText() const;

    void setChecked(bool checked);
    bool isChecked() const;

    /**
     * @brief 是否匹配,判断是否是此widget
     * @author ut000610 戴正文
     * @param type 功能键类型
     * @param key 值
     * @return
     */
    bool isEqual(ItemFuncType type, const QString &key);
    /**
     * @brief 获取焦点，显示编辑按钮, 这是键盘获取焦点
     * @author ut000610 戴正文
     */
    void getFocus();
    /**
     * @brief 丢失焦点，显示编辑按钮
     * @author ut000610 戴正文
     */
    void lostFocus();
    /**
     * @brief 重载比较 >  根据类型，文字比较大小
     * @author ut000610 戴正文
     * @param item1
     * @param item2
     * @return
     */
    friend bool operator >(const ItemWidget &item1, const ItemWidget &item2);
    /**
     * @brief 重载比较 < 根据类型，文字比较大小
     * @author ut000610 戴正文
     * @param item1
     * @param item2
     * @return
     */
    friend bool operator <(const ItemWidget &item1, const ItemWidget &item2);

public slots:
    /**
     * @brief 处理功能键点击事件: 1) 分组=>显示分组所有的项 2) 项=>修改项
     * @author ut000610 戴正文
     */
    void onFuncButtonClicked();
    /**
     * @brief 处理图标点击的事件: 1) 分组=>显示分组所有的项 2) 项=>修改项
     * @author ut000610 戴正文
     */
    void onIconButtonClicked();
    /**
     * @brief 功能键丢失焦点，当前窗口获得焦点
     * @author ut000610 戴正文
     */
    void onFocusReback();

    /**
     * @brief 焦点从功能键切出，且不在当前控件上
     * @author ut000610 戴正文
     * @param type 类型
     */
    void onFocusOut(Qt::FocusReason type);

signals:
    // 功能按钮被点击
    // 参数: item的名称 (数据的唯一值)
    // 执行项
    void itemClicked(const QString &strName);
    // 显示分组
    void groupClicked(const QString &strName, bool isFocus = false);
    // 修改项
    void itemModify(const QString &strName, bool isFocus = false);
    // 修改分组
    void groupModify(const QString &strName, bool isFocus = false);
    // 删除项
    void itemDelete(const QString &name, ItemFuncType type);
    // 焦点从控件中出
    void focusOut(Qt::FocusReason type);

protected:
    /**
     * @brief 初始化UI界面,初始化单个窗口组件的布局和空间大小
     * @author ut000610 戴正文
     */
    void initUI();
    /**
     * @brief 初始化信号槽,关联信号槽
     * @author ut000610 戴正文
     */
    void initConnections();
    /**
     * @brief 处理重绘事件:1.字体变长便短导致的问题 2. 悬浮的背景色
     * @author ut000610 戴正文
     * @param event 事件
     */
    void paintEvent(QPaintEvent *event) override;
    /**
     * @brief 进入，编辑按钮显示
     * @author ut000610 戴正文
     * @param event 事件
     */
    void enterEvent(QEvent *event) override;
    /**
     * @brief 移出，编辑按钮消失
     * @author ut000610 戴正文
     * @param event 事件
     */
    void leaveEvent(QEvent *event) override;
    /**
     * @brief 鼠标点击事件,处理item的点击事件
     * @author ut000610 戴正文
     * @param event 鼠标点击事件
     */
    void mousePressEvent(QMouseEvent *event) override;
    /**
     * @brief 处理item的点击事件
     * @author ut000610 戴正文
     * @param event 事件
     */
    void mouseReleaseEvent(QMouseEvent *event) override;
    /**
     * @brief 事件过滤
     * @author ut001121 张猛
     * @param watched
     * @param event 事件
     * @return
     */
    bool eventFilter(QObject *watched, QEvent *event) override;
    /**
     * @brief 键盘点击事件，处理右键操作
     * @author ut000610 戴正文
     * @param event 键盘点击事件
     */
    void keyPressEvent(QKeyEvent *event) override;
    /**
     * @brief 焦点进入事件
     * @author ut000610 戴正文
     * @param event 焦点进入事件
     */
    void focusInEvent(QFocusEvent *event) override;
    /**
     * @brief 焦点丢失事件
     * @author ut000610 戴正文
     * @param event 焦点丢失事件
     */
    void focusOutEvent(QFocusEvent *event) override;

private:
    // 主窗口布局
    QHBoxLayout *m_mainLayout = nullptr;
    // 图表布局
    QVBoxLayout *m_iconLayout = nullptr;
    // 文字布局
    QVBoxLayout *m_textLayout = nullptr;
    // 功能键布局
    QHBoxLayout *m_funcLayout = nullptr;

    // 勾选框
    DCheckBox   *m_checkBox = nullptr;
    // 图标
    DIconButton *m_iconButton = nullptr;
    // 文字
    DLabel *m_firstline = nullptr;
    DLabel *m_secondline = nullptr;
    // 删除键
    IconButton *m_deleteButton = nullptr;
    // 功能键
    IconButton *m_funcButton = nullptr;

    // 文字内容
    QString m_firstText = "";
    QString m_secondText = "";
    // 功能按键默认是编辑按钮
    ItemFuncType m_functType;
    // 焦点是否在窗口上 用于编辑按钮是否显示
    bool m_isFocus = false;

    /***add begin by ut001121 zhangmeng 20200924 修复BUG48618***/
    //记录触摸屏下点击时Y轴坐标
    int m_touchPressPosY = 0;
    //记录触摸屏下滑动时Y轴移动最大距离
    int m_touchSlideMaxY = 0;
    // 移动事件来源
    QObject *m_moveSource = nullptr;
    /***add end by ut001121***/

    /**
     * @brief 设置文字大小和颜色:给指定label设置初始化字体和颜色
     * @param label 标签
     * @param fontSize 字体大小
     * @param colorType 颜色
     */
    void setFont(DLabel *label, DFontSizeManager::SizeType fontSize, ItemTextColor colorType);
    /**
     * @brief 设置字体大小
     * @author ut000610 戴正文
     * @param label 标签
     * @param fontSize 字体大小
     */
    void setFontSize(DLabel *label, DFontSizeManager::SizeType fontSize);
    /**
     * @brief 设置字体颜色
     * @author ut000610 戴正文
     * @param label 标签
     * @param colorType 颜色
     */
    void setFontColor(DLabel *label, ItemTextColor colorType);
    /**
     * @brief 根据指定值，返回颜色
     * @author ut000610 戴正文
     * @param colorType 颜色
     * @return
     */
    QColor getColor(ItemTextColor colorType);
    /**
     * @brief 处理键盘事件: 右键操作，组 => 显示分组 项 => 设置焦点
     * @author ut000610 戴正文
     */
    void rightKeyPress();
    /**
     * @brief 项被点击事件：根据类型发送不同的点击信号
     * @author ut000610 戴正文
     */
    void onItemClicked();
private slots:
    /**
     * @brief 部分组件随主题颜色变化而变化
     * @param themeType 主题
     */
    void slotThemeChange(DGuiApplicationHelper::ColorType themeType);
};

#endif // ITEMWIDGET_H
