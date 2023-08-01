
// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
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
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(views)

ListView::ListView(ListType type, QWidget *parent)
    : DScrollArea(parent),
      m_type(type),
      m_mainWidget(new DWidget(this)),
      m_mainLayout(new QVBoxLayout(m_mainWidget))
{
    qCDebug(views) << "ListView constructor entered, type:" << type;
    /******** Add by ut001000 renfeixiang 2020-08-13:增加 Begin***************/
    Utils::set_Object_Name(this);
    m_mainWidget->setObjectName("ListViewMainWidget");
    m_mainLayout->setObjectName("ListViewMainLayout");
    /******** Add by ut001000 renfeixiang 2020-08-13:增加 End***************/
    // 初始化界面
    initUI();

    Service::instance()->setScrollerTouchGesture(this);
    qCDebug(views) << "ListView constructor finished";
}

ListView::~ListView()
{
    qCDebug(views) << "Enter ListView::~ListView";
    clearData();
}

void ListView::addItem(ItemFuncType type, const QString &key, const QString &strDescription)
{
    qCDebug(views) << "ListView::addItem() entered, type:" << type << "key:" << key;

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
    qCDebug(views) << "Item added successfully, new count:" << m_itemList.count();
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
    qCDebug(views) << "Enter ListView::onItemClicked";
    lostFocus(m_currentIndex);
}

inline void ListView::onFocusOut(Qt::FocusReason type)
{
    qCDebug(views) << "Enter ListView::onFocusOut";
    emit focusOut(type);
    lostFocus(m_currentIndex);
    m_focusState = false;
}

inline void ListView::onGroupClicked(const QString &strName, bool isFocus)
{
    qCDebug(views) << "Enter ListView::onGroupClicked";
    emit groupClicked(strName, isFocus);
    // 若焦点不在项上
    if (!isFocus) {
        qCDebug(views) << "Branch: isFocus is false";
        // 清空index记录
        clearIndex();
        // 焦点状态为false
        m_focusState = false;
    }
}

bool ListView::removeItem(ItemFuncType type, const QString &key)
{
    qCDebug(views) << "ListView::removeItem() entered, type:" << type << "key:" << key;

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
    qCDebug(views) << "Item removed, result:" << isFind << "new count:" << m_itemList.count();
    return isFind;
}

bool ListView::updateItem(ItemFuncType type, const QString &key, const QString &newKey, const QString &strDescription)
{
    qCDebug(views) << "ListView::updateItem() entered, type:" << type << "key:" << key << "newKey:" << newKey;

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
    qCDebug(views) << "Item updated, result:" << isFind;
    return isFind;
}

int ListView::count()
{
    // qCDebug(views) << "Enter ListView::count";
    return m_itemList.count();
}

void ListView::clearData()
{
    qCDebug(views) << "ListView::clearData() entered, item count:" << m_itemList.count();

    for (ItemWidget *item : m_itemList) {
        // 从列表中移除
        m_itemList.removeOne(item);
        // 删除项！！
        delete item;
    }
    qCDebug(views) << "All items cleared";
}

int ListView::indexFromString(const QString &key, ItemFuncType type)
{
    // qCDebug(views) << "Enter ListView::indexFromString";
    // 遍历布局
    for (int i = 0; i < m_mainLayout->count(); ++i) {
        // 获取每个窗口
        QLayoutItem *item = m_mainLayout->itemAt(i);
        ItemWidget *widget = item ? qobject_cast<ItemWidget *>(item->widget()) : nullptr;
        // 比较是否相等
        if (widget && widget->isEqual(type, key)) {
            qCDebug(views) << "Branch: widget is not null and isEqual is true";
            // 相等 返回index
            return i;
        }
    }
    qCDebug(views) << "Branch: not found, returning -1";
    // 没找到，返回-1
    return -1;
}

int ListView::currentIndex()
{
    // qCDebug(views) << "Enter ListView::currentIndex";
    return m_currentIndex;
}

int ListView::getNextIndex(int index)
{
    // qCDebug(views) << "Enter ListView::getNextIndex";
    if (index < 0) {
        qCDebug(views) << "Branch: index < 0";
        // 输入错误的index
        qCWarning(views) << "input wrong index" << index;
        return -1;
    }
    int count = m_itemList.count();
    // index小于列表最大数量
    if (index < count) {
        qCDebug(views) << "Branch: index < count";
        return index;
    }

    // index大于最大数量,且列表数量不为0
    if (index >= count && m_itemList.count() != 0) {
        qCDebug(views) << "Branch: index >= count && m_itemList.count() != 0";
        index = count - 1;
        return index;
    } else {
        // 列表没有项
        qCDebug(views) << "Branch: m_itemList.count() == 0";
        return -2;
    }
}

void ListView::setCurrentIndex(int currentIndex)
{
    qCDebug(views) << "ListView::setCurrentIndex() entered, index:" << currentIndex;

    // 首先判断currentIndex的有效性
    if (!indexIsValid(currentIndex)) {
        qCDebug(views) << "Branch: index is not valid";
        // 无效，返回
        qCInfo(views) << "index(" << currentIndex << ") is wrong";
        emit focusOut(Qt::NoFocusReason);
        m_focusState = false;
        return;
    }
    // 之前的焦点丢失
    if (currentIndex != m_currentIndex) {
        qCDebug(views) << "Branch: current index != m_currentIndex, losing focus";
        lostFocus(m_currentIndex);
    }

    // 设置焦点
    QLayoutItem *item = m_mainLayout->itemAt(currentIndex);
    ItemWidget *widget = item ? qobject_cast<ItemWidget *>(item->widget()) : nullptr;
    if (widget != nullptr) {
        qCDebug(views) << "Branch: widget is not null, setting focus";
        // 设置焦点
        qCInfo(views) << widget << "get focus" << "current index" << currentIndex;
        // Todo 让焦点不要进入主窗口
        widget->setFocus();
        m_focusState = true;
    }
    // 设置滚动条
    setScroll(currentIndex);
    m_currentIndex = currentIndex;
    qCDebug(views) << "Current index set to:" << currentIndex;
}

void ListView::clearIndex()
{
    // qCDebug(views) << "Enter ListView::clearIndex";
    m_currentIndex = -1;
}

void ListView::onItemModify(const QString &key, bool isFocusOn)
{
    qCDebug(views) << "Enter ListView::onItemModify";
    switch (m_type) {
    case ListType_Remote:
        qCDebug(views) << "Branch: ListType_Remote";
        onRemoteItemModify(key, isFocusOn);
        break;
    case ListType_Custom:
        qCDebug(views) << "Branch: ListType_Custom";
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
    qCDebug(views) << "Enter ListView::onRemoteItemModify";
    int curIndex = m_currentIndex;
    // 弹窗出，丢失焦点
    lostFocus(m_currentIndex);
    m_currentIndex = curIndex;
    // 弹窗显示
    ServerConfig *curItemServer = ServerConfigManager::instance()->getServerConfig(key);
    qCInfo(views) << "modify remote " << curItemServer->m_serverName;
    // 弹窗显示
    Service::instance()->setIsDialogShow(window(), true);
    // 根据点击事件还是键盘事件设置焦点状态
    if (isFocusOn) {
        // 键盘
        m_focusState = true;
        qCInfo(views) << "keyboard clicked";
    } else {
        // 鼠标
        qCInfo(views) << "mouse press";
        m_focusState = false;
        m_currentIndex = -1;
    }

    // 1.显示弹窗
    m_configDialog = new ServerConfigOptDlg(ServerConfigOptDlg::SCT_MODIFY, curItemServer, this);
    connect(m_configDialog, &ServerConfigOptDlg::finished, this, &ListView::onServerConfigOptDlgFinished);
    // 2. 记录弹窗
    ServerConfigManager::instance()->setModifyDialog(curItemServer->m_serverName, m_configDialog);
    m_configDialog->show();
    qCDebug(views) << "ListView::onRemoteItemModify finished";
}

inline void ListView::onServerConfigOptDlgFinished(int result)
{
    qCDebug(views) << "Enter ListView::onServerConfigOptDlgFinished";
    // 弹窗隐藏或消失
    qCInfo(views) << "focus state " << m_focusState;
    // 3. 对弹窗操作进行分析
    // 判断是否删除
    if (ServerConfigOptDlg::Accepted == result) {
        qCDebug(views) << "Branch: ServerConfigOptDlg::Accepted";
        // 判断是否需要删除
        if (m_configDialog->isDelServer()) {
            qCDebug(views) << "Branch: isDelServer is true";
            // 弹出删除弹窗
            DDialog *deleteDialog = new DDialog(tr("Delete Server"), tr("Are you sure you want to delete %1?").arg(m_configDialog->getServerName()), this);
            deleteDialog->setObjectName("RemoteDeleteDialog");
            deleteDialog->setAttribute(Qt::WA_DeleteOnClose);
            connect(deleteDialog, &DDialog::finished, this, &ListView::onDeleteServerDialogFinished);
            deleteDialog->setWindowModality(Qt::WindowModal);
            deleteDialog->setIcon(QIcon::fromTheme("deepin-terminal"));
            deleteDialog->addButton(QObject::tr("Cancel", "button"), false, DDialog::ButtonNormal);
            deleteDialog->addButton(QObject::tr("Delete", "button"), true, DDialog::ButtonWarning);
            deleteDialog->show();
            // 释放窗口
            Service::instance()->setIsDialogShow(window(), false);
        } else {
            qCDebug(views) << "Branch: ServerConfigOptDlg::NotAccepted";
            // 不删除，修改
            // 释放窗口
            Service::instance()->setIsDialogShow(window(), false);
            // 修改后会有信号刷新列表
            // 不需要删除，修改了转到这条修改的记录
            // 设置滚轮
            // 关闭后及时将弹窗删除
            // 记住修改前的位置 m_currentIndex
            qCInfo(views) << "index before modify " << m_currentIndex;
            ServerConfigManager::instance()->removeDialog(m_configDialog);
            // 刷新列表
            emit ServerConfigManager::instance()->refreshList();
            // 获取index
            int index = indexFromString(m_configDialog->getCurServer()->m_serverName);
            // 若找不到
            if (index  < 0) {
                qCDebug(views) << "Branch: index < 0, getting next index";
                // 旧位置的下一个
                index = getNextIndex(m_currentIndex);
            }
            // 依旧没有找到啦
            if (index < 0) {
                qCDebug(views) << "Branch: still no valid index found";
                qCInfo(views) << "no next item";
                if (m_focusState) {
                    qCDebug(views) << "Branch: m_focusState is true, emitting focusOut";
                    // 有焦点，焦点出
                    emit focusOut(Qt::NoFocusReason);
                }
            }
            // 找的到
            else {
                qCDebug(views) << "Branch: valid index found";
                if (m_focusState) {
                    qCDebug(views) << "Branch: m_focusState is true, setting focus";
                    // 设置焦点
                    setFocus();
                    // 有焦点，设置焦点
                    setCurrentIndex(index);
                } else {
                    qCDebug(views) << "Branch: m_focusState is false, setting scroll";
                    // 没焦点,设置滚轮
                    setScroll(index);
                }
            }
        }
    }
    // 不接受，reject 修改弹窗
    else {
        qCDebug(views) << "Branch: ServerConfigOptDlg::Rejected";
        // 释放窗口
        Service::instance()->setIsDialogShow(window(), false);
        if (m_focusState) {
            qCDebug(views) << "Branch: m_focusState is true";
            int index = indexFromString(m_configDialog->getCurServer()->m_serverName);
            setCurrentIndex(index);

            // fix bug#65109焦点在自定义编辑按钮上，enter键进入后按esc退出，焦点不在编辑按钮上
            ItemWidget *itemWidget = m_itemList.at(m_currentIndex);
            itemWidget->getFocus();
        }
        // 取消后及时将弹窗删除
        ServerConfigManager::instance()->removeDialog(m_configDialog);
    }
    qCDebug(views) << "ListView::onServerConfigOptDlgFinished finished";
}

void ListView::onDeleteServerDialogFinished(int result)
{
    qCDebug(views) << "Enter ListView::onDeleteServerDialogFinished, result:" << result;
    if (result == DDialog::Accepted && m_configDialog) {
        // 用户确认删除，调用删除处理函数
        QString serverName = m_configDialog->getServerName();
        onDeleteDialogFinished(serverName, ItemFuncType_Item);
    }
    // 释放窗口
    Service::instance()->setIsDialogShow(window(), false);
    qCDebug(views) << "Exit ListView::onDeleteServerDialogFinished";
}

void ListView::onDeleteDialogFinished(const QString &key, ItemFuncType type)
{
    qCDebug(views) << "Enter ListView::onDeleteServerDialogFinished";
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
    qCDebug(views) << "Enter ListView::onCustomItemModify";
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

    // 根据点击事件还是键盘事件设置焦点状态
    if (isFocusOn) {
        // 键盘
        m_focusState = true;
        qCDebug(views) << "Branch: isFocusOn is true";
    } else {
        // 鼠标
        m_focusState = false;
        m_currentIndex = -1;
        qCDebug(views) << "Branch: isFocusOn is false";
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

/**
 * @brief 接收 DGuiApplicationHelper::sizeModeChanged() 信号, 根据不同的布局模式调整
 *      当前界面的布局. 只能在界面创建完成后调用.
 */
void ListView::updateSizeMode()
{
    qCDebug(views) << "Enter ListView::updateSizeMode";
#ifdef DTKWIDGET_CLASS_DSizeMode
    // 遍历子项进行更新
    for (ItemWidget *item : m_itemList) {
        item->updateSizeMode();
    }
#endif
}

inline void ListView::onCustomCommandOptDlgFinished(int result)
{
    qCDebug(views) << "Enter ListView::onCustomCommandOptDlgFinished";
    // 弹窗隐藏或消失
    //Service::instance()->setIsDialogShow(window(), false);//暂时保留

    if (QDialog::Accepted == result) {
        qCDebug(views) << "Branch: QDialog::Accepted";
        //确认修改处理
        qCInfo(views) << "Modify Custom Command";
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
        if (m_focusState) {
            qCDebug(views) << "Branch: m_focusState is true, updating focus";
            // 将焦点落回
            m_currentIndex = index;
            setCurrentIndex(index);

            // 滚轮滚动到最新的位置
            setScroll(index);
            setFocus();
        }

    } else if (QDialog::Rejected == result) {
        qCDebug(views) << "Branch: QDialog::Rejected";
        //Delete custom command 删除自定义命令处理
        if (m_pdlg->isDelCurCommand()) {
            qCInfo(views) << "Delete Custom Command";
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
        qCDebug(views) << "Branch: m_focusState is true, restoring focus";
        // 将焦点落回
        setCurrentIndex(m_currentIndex);

        // 滚轮滚动到最新的位置
        setScroll(m_currentIndex);

        // fix bug#65109焦点在自定义编辑按钮上，enter键进入后按esc退出，焦点不在编辑按钮上
        if (QDialog::Rejected == result) {
            qCDebug(views) << "Branch: result is Rejected, setting focus to item widget";
            ItemWidget *itemWidget = m_itemList.at(m_currentIndex);
            itemWidget->getFocus();
        } else {
            qCDebug(views) << "Branch: result is not Rejected";
            qCInfo(views) << "QDialog accepted result is:" << result;
            if (-1 != result) {
                qCDebug(views) << "Branch: result != -1, setting focus";
                setFocus();
            } else {
                qCDebug(views) << "Branch: result == -1";
                qCInfo(views) << "QDialog Rejected";
                ItemWidget *itemWidget = m_itemList.at(m_currentIndex);
                itemWidget->getFocus();
            }
        }
    }

}

inline void ListView::onDeleteCustomCommandFinished(int result)
{
    qCDebug(views) << "Enter ListView::onDeleteCustomCommandFinished";
    //是否删除自定义命令再次确认操作
    if (QDialog::Accepted == result) {
        qCDebug(views) << "Branch: QDialog::Accepted";
        // 删除前获取位置
        int index = indexFromString(m_pdlg->m_currItemData->m_cmdName);
        ShortcutManager::instance()->delCustomCommand(*(m_pdlg->m_currItemData));
        // 删除项
        removeItem(ItemFuncType_Item, m_pdlg->m_currItemData->m_cmdName);
        emit listItemCountChange();
        // 删除后焦点位置
        if (m_focusState) {
            qCDebug(views) << "Branch: m_focusState is true";
            // 获取下一个的位置
            index = getNextIndex(index);
            if (index >= 0) {
                // 找的到
                setCurrentIndex(index);
                setScroll(index);
                setFocus();
            } else {
                qCDebug(views) << "Branch: index < 0";
                // 找不到
                qCInfo(views) << "can't find index" << index;
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
    qCDebug(views) << "Enter ListView::keyPressEvent";
    switch (event->key()) {
    case Qt::Key_Up:
        qCDebug(views) << "Branch: Qt::Key_Up";
        setFocusFromeIndex(m_currentIndex, ListFocusUp);
        break;
    case Qt::Key_Down:
        qCDebug(views) << "Branch: Qt::Key_Down";
        setFocusFromeIndex(m_currentIndex, ListFocusDown);
        break;
    case Qt::Key_Home:
        qCDebug(views) << "Branch: Qt::Key_Home";
        setFocusFromeIndex(m_currentIndex, ListFocusHome);
        break;
    case Qt::Key_End:
        qCDebug(views) << "Branch: Qt::Key_End";
        setFocusFromeIndex(m_currentIndex, ListFocusEnd);
        break;
    default:
        DScrollArea::keyPressEvent(event);
        break;
    }
}

void ListView::focusInEvent(QFocusEvent *event)
{
    // qCDebug(views) << "Enter ListView::focusInEvent";
    if (m_currentIndex == -1) {
        // qCDebug(views) << "Branch: m_currentIndex == -1, setting to 0";
        m_currentIndex = 0;
    }

    if (event->reason() == Qt::TabFocusReason) {
        qCDebug(views) << "Branch: TabFocusReason";
        setCurrentIndex(0);
    } else {
        qCDebug(views) << "Branch: not TabFocusReason";
        setCurrentIndex(m_currentIndex);
    }

    qCInfo(views) << "ListView current index : " << m_currentIndex << event->reason();
    m_focusState = true;
    DScrollArea::focusInEvent(event);
}

void ListView::setFocusState(bool focusState)
{
    // qCDebug(views) << "Enter ListView::setFocusState";
    m_focusState = focusState;
}

bool ListView::getFocusState() const
{
    // qCDebug(views) << "Enter ListView::getFocusState";
    return m_focusState;
}

void ListView::initUI()
{
    qCDebug(views) << "Enter ListView::initUI";
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

#ifdef DTKWIDGET_CLASS_DSizeMode
    qCDebug(views) << "Branch: DTKWIDGET_CLASS_DSizeMode defined";
    updateSizeMode();
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, &ListView::updateSizeMode);
#endif
    qCDebug(views) << "ListView::initUI finished";
}

void ListView::setItemIcon(ItemFuncType type, ItemWidget *item)
{
    qCDebug(views) << "Enter ListView::setItemIcon";
    switch (type) {
    // 项图标
    case ItemFuncType_Item:
        qCDebug(views) << "Branch: ItemFuncType_Item";
        if (ListType_Custom == m_type) {
            qCDebug(views) << "Branch: ListType_Custom";
            // 自定义项
            item->setIcon("dt_command");
        } else if (ListType_Remote == m_type) {
            qCDebug(views) << "Branch: ListType_Remote";
            // 远程项
            item->setIcon("dt_server");
        }
        break;
    // 远程图标
    case ItemFuncType_Group:
        qCDebug(views) << "Branch: ItemFuncType_Group";
        item->setIcon("dt_server_group");
        break;
    case ItemFuncType_UngroupedItem:
        item->setIcon("dt_server");
        break;
    }
    qCDebug(views) << "ListView::setItemIcon finished";
}

/*******************************************************************************
 1. @函数:    用于getWidgetIndex中qSort排序的比较函数
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-20
 4. @说明:    根据类型，文字比较大小
*******************************************************************************/
static inline bool comparator(ItemWidget *item1, ItemWidget *item2)
{
    // qCDebug(views) << "Enter ListView::comparator";
    // item1 item2都是组 或 item1 item2 都不是组
    if (item1->getFuncType() == item2->getFuncType()) {
        // qCDebug(views) << "Branch: item1->getFuncType() == item2->getFuncType()";
        // 比较文字
        if (QString::compare(item1->getFirstText(), item2->getFirstText()) < 0) {
            // qCDebug(views) << "Branch: text comparison < 0";
            // 小于 0 例:ab bc
            return true;
        } else {
            // qCDebug(views) << "Branch: text comparison >= 0";
            // 其他情况
            return false;
        }
    } else if (item1->getFuncType() > item2->getFuncType()) {
        // qCDebug(views) << "Branch: item1->getFuncType() > item2->getFuncType()";
        // item1 是组 1 item2不是 0
        return true;
    } else {
        // qCDebug(views) << "Branch: item1->getFuncType() < item2->getFuncType()";
        // item1 不是组 0 item2是 1
        return false;
    }
}

int ListView::getWidgetIndex(ItemWidget *itemWidget)
{
    qCDebug(views) << "Enter ListView::getWidgetIndex";
    std::sort(m_itemList.begin(), m_itemList.end(), comparator);
    // 从0开始
    int index = 0;
    int currentIndex = 0;
    // 遍历列表
    for (ItemWidget *item : m_itemList) {
        // 判断是否小于，是就将index++
        if (itemWidget->isEqual((item->getFuncType()), item->getFirstText())) {
            qCDebug(views) << "Branch: found matching item at index:" << index;
            currentIndex = index;
            break;
        }
        ++index;
    }
    qCDebug(views) << "getWidgetIndex returning:" << currentIndex;
    return currentIndex;
}

void ListView::setFocusFromeIndex(int currentIndex, ListFocusType focusType)
{
    qCDebug(views) << "Enter ListView::setFocusFromeIndex";
    int count = m_itemList.count();

    int index = currentIndex;
    if (ListFocusUp == focusType) {
        qCDebug(views) << "Branch: ListFocusUp";
        --index;
    } else if (ListFocusDown == focusType) {
        qCDebug(views) << "Branch: ListFocusDown";
        ++index;
    } else if (ListFocusHome == focusType) {
        qCDebug(views) << "Branch: ListFocusHome";
        index = 0;
    } else if (ListFocusEnd == focusType) {
        qCDebug(views) << "Branch: ListFocusEnd";
        index = this->count() - 1;
    } else {
        qCDebug(views) << "Branch: other focus type";
        qCInfo(views) << "Type of current focus:" << focusType ;
    }

    // index >= 0 < 最大数量
    // 最上
    if (index < 0) {
        qCDebug(views) << "Branch: index < 0, setting to 0";
        index = 0;
        m_scrollPostion = 0;
        // 提示音
        DBusManager::callSystemSound();
    }
    // 之前焦点所在位置丢失焦点
    if (index != currentIndex && index != count && index >= 0) {
        qCDebug(views) << "Branch: losing focus from previous item";
        // 不是到头的情况下，前一个丢失焦点
        lostFocus(currentIndex);
    }
    // 最下
    if (index == count) {
        qCDebug(views) << "Branch: index == count, adjusting";
        // 让index恢复正常
        --index;
        m_scrollPostion = verticalScrollBar()->value();
        // 提示音
        DBusManager::callSystemSound();
    } else {
        qCDebug(views) << "Branch: setting focus to widget";
        QLayoutItem *item = m_mainLayout->itemAt(index);
        ItemWidget *widget = item ? qobject_cast<ItemWidget *>(item->widget()) : nullptr;
        if (widget != nullptr) {
            qCDebug(views) << "Branch: widget is not null";
            // 设置焦点
            widget->setFocus();
            // 设置滚动条 上下移动
            if (widget->y() + widget->height() < height()) {
                qCDebug(views) << "Branch: widget height fits in view";
                verticalScrollBar()->setValue(0);
            } else {
                qCDebug(views) << "Branch: widget needs scrolling";
                const int listItemHeight = 70;
                if (ListFocusUp == focusType) {
                    qCDebug(views) << "Branch: scrolling up";
                    m_scrollPostion -= listItemHeight;
                } else if (ListFocusDown == focusType && currentIndex < count/* - 1*/) {
                    qCDebug(views) << "Branch: scrolling down";
                    m_scrollPostion += listItemHeight;
                } else if (ListFocusHome == focusType) {
                    qCDebug(views) << "Branch: scrolling to home";
                    m_scrollPostion = 0;
                } else if (ListFocusEnd == focusType) {
                    qCDebug(views) << "Branch: scrolling to end";
                    m_scrollPostion += (count - 1) * listItemHeight;
                } else {
                    qCDebug(views) << "Branch: keeping current scroll position";
                    m_scrollPostion = verticalScrollBar()->value();
                }
            }
        }
    }
    // 计算range
    int range = calculateRange(height());
    verticalScrollBar()->setRange(0, range);
    if (0 == index) {
        qCDebug(views) << "Branch: index is 0, resetting scroll position";
        m_scrollPostion = 0;
    }

    verticalScrollBar()->setValue(m_scrollPostion);
    qCInfo(views) << "up down scrollPostion : " << m_scrollPostion << verticalScrollBar()->value();

    // 需要让m_currentIndex于焦点所在位置同步
    m_currentIndex = index;
    qCInfo(views) << "current index : " << m_currentIndex;
    qCDebug(views) << "ListView::setFocusFromeIndex finished";
}

void ListView::lostFocus(int preIndex)
{
    qCDebug(views) << "Enter ListView::lostFocus";
    // 没有前一个
    if (-1 == preIndex) {
        qCDebug(views) << "Branch: preIndex is -1, returning";
        return;
    }

    QLayoutItem *item = m_mainLayout->itemAt(preIndex);
    ItemWidget *widget = item ? qobject_cast<ItemWidget *>(item->widget()) : nullptr;
    if (widget != nullptr) {
        qCDebug(views) << "Branch: widget is not null, losing focus";
        // 丢失焦点
        widget->lostFocus();
        qCInfo(views) << widget << "lost focus";
    }
    qCDebug(views) << "ListView::lostFocus finished";
}

void ListView::setScroll(int currentIndex)
{
    qCDebug(views) << "Enter ListView::setScroll";
    // 计算range
    int range = calculateRange(height());
    verticalScrollBar()->setRange(0, range);
    QLayoutItem *item = m_mainLayout->itemAt(currentIndex);
    if (item != nullptr) {
        qCDebug(views) << "Branch: item is not null";
        int postion = currentIndex  * 70;
        if ((postion + 60) < height()) {
            qCDebug(views) << "Branch: position fits in height";
            verticalScrollBar()->setValue(0);
            m_scrollPostion = 0;
        } else {
            qCDebug(views) << "Branch: position needs scrolling";
            m_scrollPostion = 0;
            // 差的距离
            int distance = postion - height();
            if (distance >= 0) {
                qCDebug(views) << "Branch: distance >= 0, calculating scroll position";
                // 倍率
                int magn = distance / 10 + 1;
                m_scrollPostion = 0 + magn * 70;
            }
            verticalScrollBar()->setValue(m_scrollPostion);
        }
    }
    qCDebug(views) << "ListView::setScroll finished";
}

bool ListView::indexIsValid(int index)
{
    // qCDebug(views) << "Enter ListView::indexIsValid";
    return ((index >= 0) && (index < m_itemList.count()));
}

int ListView::calculateRange(int height)
{
    // qCDebug(views) << "Enter ListView::calculateRange";
    int count = m_itemList.count();
    int range  = count * 70 - 10 - height;
    return range;
}

void ListView::mousePressEvent(QMouseEvent *event)
{
    // qCDebug(views) << "Enter ListView::mousePressEvent";
    Q_UNUSED(event)
    //return QScrollArea::mousePressEvent(event);
}

void ListView::mouseMoveEvent(QMouseEvent *event)
{
    // qCDebug(views) << "Enter ListView::mouseMoveEvent";
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
