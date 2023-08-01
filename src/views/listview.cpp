
// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "listview.h"
#include "service.h"
#include "serverconfigmanager.h"
#include "dbusmanager.h"
#include "customcommandoptdlg.h"
#include "serverconfigoptdlg.h"

// qt
#include <QDebug>
#include <QScroller>

ListView::ListView(ListType type, QWidget *parent)
    : DScrollArea(parent),
      m_type(type),
      m_mainWidget(new DWidget(this)),
      m_mainLayout(new QVBoxLayout(m_mainWidget))
{
    /******** Add by ut001000 renfeixiang 2020-08-13:增加 Begin***************/
    Utils::set_Object_Name(this);
    m_mainWidget->setObjectName("ListViewMainWidget");
    m_mainLayout->setObjectName("ListViewMainLayout");
    /******** Add by ut001000 renfeixiang 2020-08-13:增加 End***************/
    // 初始化界面
    initUI();

    Service::instance()->setScrollerTouchGesture(this);
}

ListView::~ListView()
{
    clearData();
}

void ListView::addItem(ItemFuncType type, const QString &key, const QString &strDescription)
{
    // 初始化项
    ItemWidget *itemWidget = new ItemWidget(type, this);
    // 设置文字
    itemWidget->setText(key, strDescription);
    // 设置图标
    setItemIcon(type, itemWidget);
    // 添加到布局
    // 远程管理的分组和项需要排序
    // 首先获得位置
    // 添加到列表
    m_itemList.append(itemWidget);
    int index = getWidgetIndex(itemWidget);
    // 添加进去
    m_mainLayout->insertWidget(index, itemWidget);
    // 列表数量变化
    emit listItemCountChange();
    // 删除列表项
    connect(itemWidget, &ItemWidget::itemDelete, this, &ListView::deleteItem);
    // 分组项被点击
    connect(itemWidget, &ItemWidget::groupClicked, this, &ListView::onGroupClicked);
    // 列表项被点击
    connect(itemWidget, &ItemWidget::itemClicked, this, &ListView::itemClicked);
    // 修改列表
    connect(itemWidget, &ItemWidget::itemModify, this, &ListView::onItemModify);
    // 修改分组
    connect(itemWidget, &ItemWidget::groupModify, this, &ListView::onGroupModify);
    // 焦点切出 Tab切出不返回
    connect(itemWidget, &ItemWidget::focusOut, this, &ListView::onFocusOut);
    // 列表被点击，编辑按钮隐藏
    connect(itemWidget, &ItemWidget::itemClicked, this, &ListView::onItemClicked);
}

inline void ListView::onItemClicked()
{
    lostFocus(m_currentIndex);
}

inline void ListView::onFocusOut(Qt::FocusReason type)
{
    emit focusOut(type);
    lostFocus(m_currentIndex);
    m_focusState = false;
}

inline void ListView::onGroupClicked(const QString &strName, bool isFocus)
{
    emit groupClicked(strName, isFocus);
    // 若焦点不在项上
    if (!isFocus) {
        // 清空index记录
        clearIndex();
        // 焦点状态为false
        m_focusState = false;
    }
}

bool ListView::removeItem(ItemFuncType type, const QString &key)
{
    bool isFind = false;
    for (ItemWidget *item : m_itemList) {
        // 匹配选项
        if (item->isEqual(type, key)) {
            // 删除此项
            m_itemList.removeOne(item);
            m_mainLayout->removeWidget(item);
            delete item;
            // 找到设为true
            isFind = true;
            // 列表数量变化
            emit listItemCountChange();
        }
    }
    return isFind;
}

bool ListView::updateItem(ItemFuncType type, const QString &key, const QString &newKey, const QString &strDescription)
{
    bool isFind = false;
    // 遍历，找到需要修改的项，修改
    // 组的修改是，更新数量
    // 项的修改是修改内容
    for (ItemWidget *item : m_itemList) {
        // 匹配选项
        if (item->isEqual(type, key)) {
            //更新
            item->setText(newKey, strDescription);
            // 找到设为true
            isFind = true;
        }
    }
    // 返回更新结果
    return isFind;
}

int ListView::count()
{
    return m_itemList.count();
}

void ListView::clearData()
{
    for (ItemWidget *item : m_itemList) {
        // 从列表中移除
        m_itemList.removeOne(item);
        // 删除项！！
        delete item;
    }
}

int ListView::indexFromString(const QString &key, ItemFuncType type)
{
    // 遍历布局
    for (int i = 0; i < m_mainLayout->count(); ++i) {
        // 获取每个窗口
        QLayoutItem *item = m_mainLayout->itemAt(i);
        ItemWidget *widget = item ? qobject_cast<ItemWidget *>(item->widget()) : nullptr;
        // 比较是否相等
        if (widget && widget->isEqual(type, key)) {
            // 相等 返回index
            return i;
        }
    }
    // 没找到，返回-1
    return -1;
}

int ListView::currentIndex()
{
    return m_currentIndex;
}

int ListView::getNextIndex(int index)
{
    if (index < 0) {
        // 输入错误的index
        qInfo() << __FUNCTION__ << "input wrong index" << index;
        return -1;
    }
    int count = m_itemList.count();
    // index小于列表最大数量
    if (index < count)
        return index;

    // index大于最大数量,且列表数量不为0
    if (index >= count && m_itemList.count() != 0) {
        index = count - 1;
        return index;
    } else {
        // 列表没有项
        return -2;
    }
}

void ListView::setCurrentIndex(int currentIndex)
{
    // 首先判断currentIndex的有效性
    if (!indexIsValid(currentIndex)) {
        // 无效，返回
        qInfo() << "index : " << currentIndex << " is wrong";
        emit focusOut(Qt::NoFocusReason);
        m_focusState = false;
        return;
    }
    qInfo() << __FUNCTION__ << currentIndex;
    // 之前的焦点丢失
    if (currentIndex != m_currentIndex)
        lostFocus(m_currentIndex);

    // 设置焦点
    QLayoutItem *item = m_mainLayout->itemAt(currentIndex);
    ItemWidget *widget = item ? qobject_cast<ItemWidget *>(item->widget()) : nullptr;
    if (widget != nullptr) {
        // 设置焦点
        qInfo() << widget << "get focus" << "current index" << currentIndex;
        // Todo 让焦点不要进入主窗口
        widget->setFocus();
        m_focusState = true;
    }
    // 设置滚动条
    setScroll(currentIndex);
    m_currentIndex = currentIndex;
}

void ListView::clearIndex()
{
    m_currentIndex = -1;
    qInfo() << __FUNCTION__ << "clear index to  -1";
}

void ListView::onItemModify(const QString &key, bool isFocusOn)
{
    switch (m_type) {
    case ListType_Remote:
        onRemoteItemModify(key, isFocusOn);
        break;
    case ListType_Custom:
        onCustomItemModify(key, isFocusOn);
        break;
    }
}

void ListView::onGroupModify(const QString &key, bool isFocusOn)
{
    emit groupModify(key);
    // 若焦点不在项上
    if (!isFocusOn) {
        // 清空index记录
        clearIndex();
        // 焦点状态为false
        m_focusState = false;
    }
}

void ListView::onRemoteItemModify(const QString &key, bool isFocusOn)
{
    int curIndex = m_currentIndex;
    // 弹窗出，丢失焦点
    lostFocus(m_currentIndex);
    m_currentIndex = curIndex;
    // 弹窗显示
    ServerConfig *curItemServer = ServerConfigManager::instance()->getServerConfig(key);
    qInfo() << __FUNCTION__ << "modify remote " << curItemServer->m_serverName;
    // 弹窗显示
    Service::instance()->setIsDialogShow(window(), true);
    // 根据点击事件还是键盘事件设置焦点状态
    if (isFocusOn) {
        // 键盘
        m_focusState = true;
        qInfo() << "keyboard clicked";
    } else {
        // 鼠标
        qInfo() << "mouse press";
        m_focusState = false;
        m_currentIndex = -1;
    }

    // 1.显示弹窗
    m_configDialog = new ServerConfigOptDlg(ServerConfigOptDlg::SCT_MODIFY, curItemServer, this);
    connect(m_configDialog, &ServerConfigOptDlg::finished, this, &ListView::onServerConfigOptDlgFinished);
    // 2. 记录弹窗
    ServerConfigManager::instance()->setModifyDialog(curItemServer->m_serverName, m_configDialog);
    m_configDialog->show();
}

inline void ListView::onServerConfigOptDlgFinished(int result)
{
    // 弹窗隐藏或消失
    qInfo() << "focus state " << m_focusState;
    // 3. 对弹窗操作进行分析
    // 判断是否删除
    if (ServerConfigOptDlg::Accepted == result) {
        // 释放窗口
        Service::instance()->setIsDialogShow(window(), false);
        // 修改后会有信号刷新列表
        // 不需要删除，修改了转到这条修改的记录
        // 设置滚轮
        // 关闭后及时将弹窗删除
        // 记住修改前的位置 m_currentIndex
        qInfo() << "index before modify " << m_currentIndex;
        ServerConfigManager::instance()->removeDialog(m_configDialog);
        // 刷新列表
        emit ServerConfigManager::instance()->refreshList();
        // 获取index
        int index = indexFromString(m_configDialog->getCurServer()->m_serverName);
        // 若找不到
        if (index  < 0) {
            // 旧位置的下一个
            index = getNextIndex(m_currentIndex);
        }
        // 依旧没有找到啦
        if (index < 0) {
            qInfo() << "no next item";
            if (m_focusState) {
                // 有焦点，焦点出
                emit focusOut(Qt::NoFocusReason);
            }
        }
        // 找的到
        else {
            if (m_focusState) {
                // 设置焦点
                setFocus();
                // 有焦点，设置焦点
                setCurrentIndex(index);
            } else {
                // 没焦点,设置滚轮
                setScroll(index);
            }
        }
    }
    // 不接受，reject 修改弹窗
    else {
        // 释放窗口
        Service::instance()->setIsDialogShow(window(), false);
        if (m_focusState) {
            int index = indexFromString(m_configDialog->getCurServer()->m_serverName);
            setCurrentIndex(index);

            // fix bug#65109焦点在自定义编辑按钮上，enter键进入后按esc退出，焦点不在编辑按钮上
            ItemWidget *itemWidget = m_itemList.at(m_currentIndex);
            itemWidget->getFocus();
        }
        // 取消后及时将弹窗删除
        ServerConfigManager::instance()->removeDialog(m_configDialog);
    }
}

void ListView::onDeleteDialogFinished(const QString &key, ItemFuncType type)
{
    // 获取index
    int index = indexFromString(key);
    // 关闭所有相关弹窗
    ServerConfigManager::instance()->closeAllDialog(key);
    if (ItemFuncType_Item == type) {
        if (ListType_Remote == m_type) {
            ServerConfigManager::instance()->delServerConfig(key);
        }
        else {
            QAction *itemAction = ShortcutManager::instance()->findActionByKey(key);
            CustomCommandData itemData;
            itemData.m_cmdName = itemAction->text();
            itemData.m_cmdText = itemAction->data().toString();
            itemData.m_cmdShortcut = itemAction->shortcut().toString();

            ShortcutManager::instance()->delCustomCommand(itemData);
            emit Service::instance()->refreshCommandPanel(itemData.m_cmdName, itemData.m_cmdName);
        }
    } else if (ItemFuncType_Group == type) {
        ServerConfigManager::instance()->delServerGroupConfig(key);
    }
    emit ServerConfigManager::instance()->refreshList();
    emit listItemCountChange();

    // Todo : 焦点返回下一个
    index = getNextIndex(index);
    if (m_focusState) {
        // 根据返回值判断焦点位置
        if (index >= 0) {
            // 回到列表,仅回到大的列表，没有回到具体的哪个点
            setFocus();
            // 列表不为空
            setCurrentIndex(index);
        } else {
            // 列表为空焦点自动往下
            m_focusState = false;
            emit focusOut(Qt::NoFocusReason);
        }
    }
}

void ListView::onCustomItemModify(const QString &key, bool isFocusOn)
{
    if (m_pdlg) {
        delete m_pdlg;
        m_pdlg = nullptr;
    }

    int curIndex = m_currentIndex;
    // 弹窗出，丢失焦点
    lostFocus(m_currentIndex);
    m_currentIndex = curIndex;

    // 弹窗显示
    Service::instance()->setIsDialogShow(window(), true);

    qInfo() << "this->count()=" << this->count();
    // 根据点击事件还是键盘事件设置焦点状态
    if (isFocusOn) {
        // 键盘
        m_focusState = true;
    } else {
        // 鼠标
        m_focusState = false;
        m_currentIndex = -1;
    }

    // 获取数据
    QAction *itemAction = ShortcutManager::instance()->findActionByKey(key);
    CustomCommandData itemData;
    itemData.m_cmdName = itemAction->text();
    itemData.m_cmdText = itemAction->data().toString();
    itemData.m_cmdShortcut = itemAction->shortcut().toString();

    m_pdlg = new CustomCommandOptDlg(CustomCommandOptDlg::CCT_MODIFY, &itemData, this);
    connect(m_pdlg, &CustomCommandOptDlg::finished, this, &ListView::onCustomCommandOptDlgFinished);
    m_pdlg->show();
}

inline void ListView::onCustomCommandOptDlgFinished(int result)
{
    int tempResult = result;

    // 弹窗隐藏或消失
    //Service::instance()->setIsDialogShow(window(), false);//暂时保留

    if (QDialog::Accepted == result) {
        //确认修改处理
        qInfo() <<  __FUNCTION__ << __LINE__ << ":mod Custom Command";
        QAction *newAction = m_pdlg->getCurCustomCmd();
        CustomCommandData itemData = *(m_pdlg->m_currItemData);
        CustomCommandData itemDel = itemData;

        itemData.m_cmdName = newAction->text();
        itemData.m_cmdText = newAction->data().toString();
        itemData.m_cmdShortcut = newAction->shortcut().toString();

        //newAction->setData(newAction->data());
        //newAction->setShortcut(newAction->shortcut());

        ShortcutManager::instance()->delCustomCommand(itemDel);
        ShortcutManager::instance()->addCustomCommand(*newAction);
        // 更新列表项
        //updateItem(ItemFuncType_Item, itemData.m_cmdName, itemData.m_cmdShortcut);
        updateItem(ItemFuncType_Item, itemDel.m_cmdName, itemData.m_cmdName);
        // 找到值
        m_pdlg->closeRefreshDataConnection();
        emit Service::instance()->refreshCommandPanel(itemDel.m_cmdName, itemData.m_cmdName);

        int index = indexFromString(itemData.m_cmdName);
        qInfo() << "-------------------------------------------index=" << index << ",itemData.m_cmdName=" << itemData.m_cmdName;
        if (m_focusState) {
            // 将焦点落回
            m_currentIndex = index;
            setCurrentIndex(index);

            // 滚轮滚动到最新的位置
            setScroll(index);
            setFocus();
        }

    } else if (QDialog::Rejected == result) {

        //Delete custom command 删除自定义命令处理
        if (m_pdlg->isDelCurCommand()) {
            qInfo() <<  __FUNCTION__ << __LINE__ << ":del Custom Command";
            DDialog *dlgDelete = new DDialog(this);
            dlgDelete->setObjectName("CustomDeleteDialog");
            dlgDelete->setAttribute(Qt::WA_DeleteOnClose);
            dlgDelete->setWindowModality(Qt::WindowModal);
            m_pdlg->m_dlgDelete = dlgDelete;

            dlgDelete->setIcon(QIcon::fromTheme("deepin-terminal"));
            dlgDelete->setTitle(tr("Are you sure you want to delete %1?").arg(m_pdlg->m_currItemData->m_cmdName));
            dlgDelete->addButton(QObject::tr("Cancel", "button"), false, DDialog::ButtonNormal);
            dlgDelete->addButton(QObject::tr("Confirm", "button"), true, DDialog::ButtonWarning);
            connect(dlgDelete, &DDialog::finished, this, &ListView::onDeleteCustomCommandFinished);
            dlgDelete->show();
        }
    }

    Service::instance()->setIsDialogShow(window(), false);

    if (m_focusState) {
        // 将焦点落回
        setCurrentIndex(m_currentIndex);

        // 滚轮滚动到最新的位置
        setScroll(m_currentIndex);

        // fix bug#65109焦点在自定义编辑按钮上，enter键进入后按esc退出，焦点不在编辑按钮上
        if (QDialog::Rejected == result) {
            qInfo() << "QDialog::Rejected";
            ItemWidget *itemWidget = m_itemList.at(m_currentIndex);
            itemWidget->getFocus();
        } else {
            qInfo() << "QDialog::Accepted result is:" << result;
            if (-1 != result) {
                setFocus();
            } else {
                qInfo() << "QDialog::Rejected";
                ItemWidget *itemWidget = m_itemList.at(m_currentIndex);
                itemWidget->getFocus();
            }
        }
    }

    qInfo() << "================================tempResult=" << tempResult;
}

inline void ListView::onDeleteCustomCommandFinished(int result)
{
    //是否删除自定义命令再次确认操作
    if (QDialog::Accepted == result) {
        // 删除前获取位置
        int index = indexFromString(m_pdlg->m_currItemData->m_cmdName);
        ShortcutManager::instance()->delCustomCommand(*(m_pdlg->m_currItemData));
        // 删除项
        removeItem(ItemFuncType_Item, m_pdlg->m_currItemData->m_cmdName);
        emit listItemCountChange();
        // 删除后焦点位置
        if (m_focusState) {
            // 获取下一个的位置
            index = getNextIndex(index);
            if (index >= 0) {
                // 找的到
                setCurrentIndex(index);
                setScroll(index);
                setFocus();
            } else {
                // 找不到
                qInfo() << "can't find index" << index;
                if (m_itemList.count() == 0) {
                    emit focusOut(Qt::NoFocusReason);
                }
            }
        }
        m_pdlg->closeRefreshDataConnection();
        emit Service::instance()->refreshCommandPanel(m_pdlg->m_currItemData->m_cmdName, m_pdlg->m_currItemData->m_cmdName);
    }
}

void ListView::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Up:
        setFocusFromeIndex(m_currentIndex, ListFocusUp);
        break;
    case Qt::Key_Down:
        setFocusFromeIndex(m_currentIndex, ListFocusDown);
        break;
    case Qt::Key_Home:
        setFocusFromeIndex(m_currentIndex, ListFocusHome);
        break;
    case Qt::Key_End:
        setFocusFromeIndex(m_currentIndex, ListFocusEnd);
        break;
    default:
        DScrollArea::keyPressEvent(event);
        break;
    }
}

void ListView::focusInEvent(QFocusEvent *event)
{
    qInfo() << __FUNCTION__ << event->reason();
    if (m_currentIndex == -1)
        m_currentIndex = 0;

    if (event->reason() == Qt::TabFocusReason)
        setCurrentIndex(0);
    else
        setCurrentIndex(m_currentIndex);

    qInfo() << "ListView current index : " << m_currentIndex << event->reason();
    m_focusState = true;
    DScrollArea::focusInEvent(event);
}

void ListView::setFocusState(bool focusState)
{
    m_focusState = focusState;
}

bool ListView::getFocusState() const
{
    return m_focusState;
}

void ListView::initUI()
{
    // 间隔
    setContentsMargins(0, 0, 0, 0);
    //布局
    m_mainLayout->setContentsMargins(10, 0, 10, 0);
    m_mainLayout->setSpacing(10);
    m_mainLayout->addStretch();
    // 焦点策略
    setFocusPolicy(Qt::TabFocus);

    setFrameShape(QFrame::NoFrame);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setSizeAdjustPolicy(QScrollArea::AdjustIgnored);
    setWidgetResizable(true);
    verticalScrollBar()->setValue(0);

    m_mainWidget->setLayout(m_mainLayout);
    setWidget(m_mainWidget);
}

void ListView::setItemIcon(ItemFuncType type, ItemWidget *item)
{
    switch (type) {
    // 项图标
    case ItemFuncType_Item:
        if (ListType_Custom == m_type) {
            // 自定义项
            item->setIcon("dt_command");
        } else if (ListType_Remote == m_type) {
            // 远程项
            item->setIcon("dt_server");
        }
        break;
    // 远程图标
    case ItemFuncType_Group:
        item->setIcon("dt_server_group");
        break;
    case ItemFuncType_UngroupedItem:
        item->setIcon("dt_server");
        break;
    }
}

/*******************************************************************************
 1. @函数:    用于getWidgetIndex中qSort排序的比较函数
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-20
 4. @说明:    根据类型，文字比较大小
*******************************************************************************/
static inline bool comparator(ItemWidget *item1, ItemWidget *item2)
{
    // item1 item2都是组 或 item1 item2 都不是组
    if (item1->getFuncType() == item2->getFuncType()) {
        // 比较文字
        if (QString::compare(item1->getFirstText(), item2->getFirstText()) < 0) {
            // 小于 0 例:ab bc
            return true;
        } else {
            // 其他情况
            return false;
        }
    } else if (item1->getFuncType() > item2->getFuncType()) {
        // item1 是组 1 item2不是 0
        return true;
    } else {
        // item1 不是组 0 item2是 1
        return false;
    }
}

int ListView::getWidgetIndex(ItemWidget *itemWidget)
{
    qSort(m_itemList.begin(), m_itemList.end(), comparator);
    // 从0开始
    int index = 0;
    int currentIndex = 0;
    // 遍历列表
    for (ItemWidget *item : m_itemList) {
        // 判断是否小于，是就将index++
        if (itemWidget->isEqual((item->getFuncType()), item->getFirstText())) {
            currentIndex = index;
            break;
        }
        ++index;
    }
    return currentIndex;
}

void ListView::setFocusFromeIndex(int currentIndex, ListFocusType focusType)
{
    int count = m_itemList.count();

    int index = currentIndex;
    if (ListFocusUp == focusType)
        --index;
    else if (ListFocusDown == focusType)
        ++index;
    else if (ListFocusHome == focusType)
        index = 0;
    else if (ListFocusEnd == focusType)
        index = this->count() - 1;
    else
        qInfo() << "index:" << index << endl;

    qInfo() << "focus index:" << index << endl;

    // index >= 0 < 最大数量
    // 最上
    if (index < 0) {
        index = 0;
        m_scrollPostion = 0;
        // 提示音
        DBusManager::callSystemSound();
    }
    // 之前焦点所在位置丢失焦点
    if (index != currentIndex && index != count && index >= 0) {
        // 不是到头的情况下，前一个丢失焦点
        lostFocus(currentIndex);
    }
    // 最下
    if (index == count) {
        // 让index恢复正常
        --index;
        m_scrollPostion = verticalScrollBar()->value();
        // 提示音
        DBusManager::callSystemSound();
    } else {
        QLayoutItem *item = m_mainLayout->itemAt(index);
        ItemWidget *widget = item ? qobject_cast<ItemWidget *>(item->widget()) : nullptr;
        if (widget != nullptr) {
            // 设置焦点
            widget->setFocus();
            int widget_y1 = widget->y();
            int widget_y2 = widget->y() + widget->height();
            qInfo() << "y1" << widget_y1;
            qInfo() << "y2" << widget_y2;
            qInfo() << "height" << height();
            // 设置滚动条 上下移动
            if (widget->y() + widget->height() < height()) {
                verticalScrollBar()->setValue(0);
            } else {
                const int listItemHeight = 70;
                if (ListFocusUp == focusType) {
                    m_scrollPostion -= listItemHeight;
                } else if (ListFocusDown == focusType && currentIndex < count/* - 1*/) {
                    m_scrollPostion += listItemHeight;
                } else if (ListFocusHome == focusType) {
                    m_scrollPostion = 0;
                } else if (ListFocusEnd == focusType) {
                    m_scrollPostion += (count - 1) * listItemHeight;
                } else {
                    m_scrollPostion = verticalScrollBar()->value();
                    qInfo() << "m_scrollPostion" << m_scrollPostion << endl;
                }
            }
        }
    }
    // 计算range
    int range = calculateRange(height());
    verticalScrollBar()->setRange(0, range);
    if (0 == index)
        m_scrollPostion = 0;

    verticalScrollBar()->setValue(m_scrollPostion);
    qInfo() << "up down scrollPostion : " << m_scrollPostion << verticalScrollBar()->value();

    // 需要让m_currentIndex于焦点所在位置同步
    m_currentIndex = index;
    qInfo() << "current index : " << m_currentIndex;
}

void ListView::lostFocus(int preIndex)
{
    // 没有前一个
    if (-1 == preIndex)
        return;

    QLayoutItem *item = m_mainLayout->itemAt(preIndex);
    ItemWidget *widget = item ? qobject_cast<ItemWidget *>(item->widget()) : nullptr;
    if (widget != nullptr) {
        // 丢失焦点
        widget->lostFocus();
        qInfo() << widget << "lost focus";
    }
}

void ListView::setScroll(int currentIndex)
{
    // 计算range
    int range = calculateRange(height());
    verticalScrollBar()->setRange(0, range);
    QLayoutItem *item = m_mainLayout->itemAt(currentIndex);
    if (item != nullptr) {
        int postion = currentIndex  * 70;
        qInfo() << "postion : " << postion;
        if ((postion + 60) < height()) {
            verticalScrollBar()->setValue(0);
            m_scrollPostion = 0;
        } else {
            m_scrollPostion = 0;
            // 差的距离
            int distance = postion - height();
            if (distance >= 0) {
                // 倍率
                int magn = distance / 10 + 1;
                m_scrollPostion = 0 + magn * 70;
            }
            verticalScrollBar()->setValue(m_scrollPostion);
        }
        qInfo() << "scrollPostion " << m_scrollPostion;
    }
}

bool ListView::indexIsValid(int index)
{
    qInfo() << index << m_itemList.count();
    return ((index >= 0) && (index < m_itemList.count()));
}

int ListView::calculateRange(int height)
{
    int count = m_itemList.count();
    int range  = count * 70 - 10 - height;
    return range;
}

void ListView::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    //return QScrollArea::mousePressEvent(event);
}

void ListView::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    //return QScrollArea::mouseMoveEvent(event);
}

void ListView::deleteItem(const QString &key, ItemFuncType type)
{
    // 弹出删除弹窗
    QString title;
    switch(type) {
    case ItemFuncType_Item:
        if (ListType_Remote == m_type)
            title = tr("Delete Server");
        else
            title = tr("Delete Custom Command");
        break;
    case ItemFuncType_Group:
        title = tr("Cancel Server Group");
        break;
    }
    QString alertText;
    if (type == ItemFuncType_Item) {
        alertText = tr("Are you sure you want to delete %1?").arg(key);
    } else {
        alertText = tr("Ungrouped servers will go back to server list!");
    }
    DDialog *deleteDialog = new DDialog(title, alertText, this);
    deleteDialog->setObjectName("RemoteDeleteDialog");
    deleteDialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(deleteDialog, &DDialog::finished, this, [this, key, type] (int result) {
        if (result == DDialog::Accepted)
            onDeleteDialogFinished(key, type);
    });
    deleteDialog->setWindowModality(Qt::WindowModal);
    deleteDialog->setIcon(QIcon::fromTheme("deepin-terminal"));
    deleteDialog->addButton(QObject::tr("Cancel", "button"), false, DDialog::ButtonNormal);
    deleteDialog->addButton(QObject::tr("Delete", "button"), true, DDialog::ButtonWarning);
    deleteDialog->show();
    // 释放窗口
    Service::instance()->setIsDialogShow(window(), false);
}
