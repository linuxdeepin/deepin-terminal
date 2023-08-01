// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/*******************************************************************************
 1. @类名:    ListView
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    列表控件
             提供添加、删除和更新列表项和选择列表项焦点的功能
             根据自定义或远程设置不同的列表展示
*******************************************************************************/
#ifndef LISTVIEW_H
#define LISTVIEW_H
#include "itemwidget.h"

// dtk
#include <DWidget>
#include <DScrollArea>

// qt
#include <QVBoxLayout>
#include <QList>
#include <QKeyEvent>
#include <QFocusEvent>
#include <QScrollBar>
#include <QScrollArea>
#include <QWheelEvent>

DWIDGET_USE_NAMESPACE

enum ListType {
    // 自定义
    ListType_Custom,
    // 远程
    ListType_Remote
};

enum ListFocusType {
    ListFocusUp,
    ListFocusDown,
    ListFocusHome,
    ListFocusEnd,
    ListFocusPageUp,
    ListFocusPageDown
};

class ServerConfigOptDlg;
class CustomCommandOptDlg;
class ListView : public DScrollArea
{
    Q_OBJECT
public:
    ListView(ListType type, QWidget *parent = nullptr);
    /**
     * @brief 析构，用于释放列表中的数据
     * @author ut000610 戴正文
     */
    ~ListView() override;

    /**
     * @brief 根据所给信息添加项
     * @author ut000610 戴正文
     * @param type 可执行的项
     * @param key
     * @param strDescription
     */
    void addItem(ItemFuncType type, const QString &key, const QString &strDescription = "");
    /**
     * @brief 根据类型和key值移除项
     * @author ut000610 戴正文
     * @param type 类型
     * @param key key值
     * @return
     */
    bool removeItem(ItemFuncType type, const QString &key);
    /**
     * @brief 针对列表中的一项进行更新
     * @author ut000610 戴正文
     * @param type
     * @param key
     * @param newKey
     * @param strDescription 新key值
     * @return
     */
    bool updateItem(ItemFuncType type, const QString &key, const QString &newKey, const QString &strDescription = "");
    /**
     * @brief 获取列表中项的数量
     * @author ut000610 戴正文
     * @return
     */
    int count();
    /**
     * @brief 清空数据,刷新列表需要清空数据
     * @author ut000610 戴正文
     */
    void clearData();
    /**
     * @brief 根据名称获取现有列表的index
     * @author ut000610 戴正文
     * @param key
     * @param type
     * @return
     */
    int indexFromString(const QString &key, ItemFuncType type = ItemFuncType_Item);
    /**
     * @brief 返回焦点选中的index
     * @author ut000610 戴正文
     * @return
     */
    int currentIndex();
    /**
     * @brief 获取当前下一个的index 实质判断现在此index是否可用
     * @author ut000610 戴正文
     * @param index
     * @return
     */
    int getNextIndex(int index);
    /**
     * @brief 设置当前焦点
     * @author ut000610 戴正文
     * @param currentIndex 当前index
     */
    void setCurrentIndex(int currentIndex);
    /**
     * @brief 清空焦点 清空index
     * @author ut000610 戴正文
     */
    void clearIndex();
    /**
     * @brief 根据index切换焦点
     * @author ut000610 戴正文
     * @param currentIndex 当前项的值
     * @param focusType:ListFocusUp,向上;ListFocusDown, 向下;ListFocusHome, 第一个;ListFocusEnd,  最后一个;ListFocusPageUp, 向上翻页;ListFocusPageDown 向下翻页
     */
    void setFocusFromeIndex(int currentIndex, ListFocusType focusType);
    /**
     * @brief 因为上下键或者Tab丢失焦点
     * @author ut000610 戴正文
     * @param preIndex 丢失焦点前的项
     */
    void lostFocus(int preIndex);
    /**
     * @brief 将滚轮设置到当前项
     * @author ut000610 戴正文
     * @param currentIndex 当前项的值
     */
    void setScroll(int currentIndex);
    /**
     * @brief 获得焦点状态
     * @author ut000610 戴正文
     * @return
     */
    bool getFocusState() const;
    /**
     * @brief 设置焦点状态
     * @author ut000610 戴正文
     * @param focusState
     */
    void setFocusState(bool focusState);
    /**
     * @brief 添加项到布局,根据类型进行数据的插入位置的选择，按type和key值，以及strDescription排序
     * @author ut000610 戴正文
     * @param itemWidget
     * @return
     */
    int getWidgetIndex(ItemWidget *itemWidget);
    const QList<ItemWidget*> itemList() { return m_itemList; }

public slots:
    /**
     * @brief 表项被修改，弹出弹窗给用户修改
     * @param key Item项
     * @param isFocusOn 是否有焦点
     */
    void onItemModify(const QString &key, bool isFocusOn);
    void onGroupModify(const QString &key, bool isFocusOn);

private slots:
    /**
     * @brief 焦点切出
     * @author ut000438 王亮
     * @param type
     */
    void onFocusOut(Qt::FocusReason type);
    /**
     * @brief 列表项点击事件
     * @author ut000438 王亮
     */
    void onItemClicked();
    /**
     * @brief 分组项被点击
     * @author
     * @param strName 分组项名称
     * @param isFocus 是否有焦点
     */
    void onGroupClicked(const QString &strName, bool isFocus = false);

    /**
     * @brief 编辑自定义命令弹窗finished
     * @author ut000438 王亮
     * @param result
     */
    void onCustomCommandOptDlgFinished(int result);
    /**
     * @brief 删除自定义命令弹窗finished
     * @author ut000438 王亮
     * @param result
     */
    void onDeleteCustomCommandFinished(int result);

    /**
     * @brief 编辑远程服务器弹窗finished
     * @author ut000438 王亮
     * @param result
     */
    void onServerConfigOptDlgFinished(int result);
    /**
     * @brief 删除远程服务器弹窗finished
     * @author ut000438 王亮
     * @param result
     */
    void onDeleteDialogFinished(const QString &key, ItemFuncType type);

signals:
    // 列表数量变化
    void listItemCountChange();
    // 列表项被点击
    void itemClicked(const QString &key);
    // 分组项被点击
    void groupClicked(const QString &key, bool isFocus);
    // 列表项要被修改
    void itemModify(const QString &key);
    // 分组要被编辑
    void groupModify(const QString &key);
    // 焦点Tab出
    void focusOut(Qt::FocusReason type);

protected:
    /**
     * @brief 键盘事件 对Tab和上下键进行处理
     * @author ut000610 戴正文
     * @param event 键盘事件
     */
    void keyPressEvent(QKeyEvent *event) override;
    /**
     * @brief 焦点进入事件
     * @author ut000610 戴正文
     * @param event 焦点事件
     */
    void focusInEvent(QFocusEvent *event) override;
    /**
     * @brief 鼠标移动事件
     * @author ut000610 戴正文
     */
    void mouseMoveEvent(QMouseEvent *) override;
    /**
     * @brief 鼠标按下事件
     * @author ut000610 戴正文
     */
    void mousePressEvent(QMouseEvent *) override;

private:
    // 列表类型
    ListType m_type;
    // 焦点切出还是进入 true 焦点在  false 焦点不在
    bool m_focusState = false;
    // 当前焦点
    int m_currentIndex = -1;
    // 当前滚轮位置
    int m_scrollPostion = 0;
    // 幕布
    DWidget *m_mainWidget = nullptr;
    // 远程弹窗
    ServerConfigOptDlg *m_configDialog = nullptr;
    // 自定义弹窗
    CustomCommandOptDlg *m_pdlg = nullptr;
    // 主布局
    QVBoxLayout *m_mainLayout = nullptr;
    // 列表项数据
    QList<ItemWidget *> m_itemList;

    /**
     * @brief 初始化UI界面,初始化列表，设置列表内布局，垂直布局，控件间隔10px
     * @author ut000610 戴正文
     */
    void initUI();
    /**
     * @brief 根据列表类型设置项图标,远程组图标 dt_server_group 远程服务器图标 dt_server 自定义图标 dt_command
     * @author ut000610 戴正文
     * @param type 列表类型
     * @param item 列表项
     */
    void setItemIcon(ItemFuncType type, ItemWidget *item);
    /**
     * @brief 判断index的有效性
     * @author ut000610 戴正文
     * @param index
     * @return
     */
    bool indexIsValid(int index);
    /**
     * @brief 计算滚动条的range
     * @author ut000610 戴正文
     * @param height 高度
     * @return
     */
    int calculateRange(int height);
    /**
     * @brief 理远程管理修改项的弹窗操作
     * @author ut000610 戴正文
     * @param key 修改项
     * @param isFocusOn 是否有焦点
     */
    void onRemoteItemModify(const QString &key, bool isFocusOn);
    /**
     * @brief 自定义修改项 修改自定义项处理弹窗的操作
     * @author ut000610 戴正文
     * @param key 修改项
     * @param isFocusOn 是否有焦点
     */
    void onCustomItemModify(const QString &key, bool isFocusOn);

    void deleteItem(const QString &key, ItemFuncType type);
};

#endif // LISTVIEW_H
