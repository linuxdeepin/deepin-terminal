#include "customcommandlist.h"
#include "customcommanddelegate.h"
#include "customcommanditem.h"
#include "customcommandoptdlg.h"
#include "operationconfirmdlg.h"
#include "shortcutmanager.h"

#include <DMessageBox>

#include <QAction>
#include <QDebug>

CustomCommandList::CustomCommandList(QWidget *parent) : DListView(parent)
{
    setBackgroundRole(QPalette::NoRole);
    setAutoFillBackground(false);

//    setVerticalScrollMode(ScrollPerItem);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    initData();
}

void CustomCommandList::initData()
{
    m_cmdListModel = new CustomCommandItemModel(this);
    m_cmdDelegate = new CustomCommandDelegate(this);
    this->setModel(m_cmdListModel);
    this->setItemDelegate(m_cmdDelegate);
    refreshCommandListData("");
}

void CustomCommandList::refreshCommandListData(const QString &strFilter)
{
    qDebug() << __FUNCTION__ << strFilter;
    m_cmdListModel->clear();
    m_cmdItemDataList.clear();

    QList<QAction *> &customCommandActionList = ShortcutManager::instance()->getCustomCommandActionList();
    qDebug() << "size:" << customCommandActionList.size();

    if (strFilter.isEmpty()) {
        for (int i = 0; i < customCommandActionList.size(); i++) {
            QAction *curAction = customCommandActionList[i];
            QString strCmdName = curAction->text();
            QString strCmdText = curAction->data().toString();
            QKeySequence keySeq = curAction->shortcut();
            QString strKeySeq = keySeq.toString();

            CustomCommandItemData itemData;
            itemData.m_cmdName = strCmdName;
            itemData.m_cmdText = strCmdText;
            itemData.m_cmdShortcut = strKeySeq;
            itemData.m_customCommandAction = curAction;

            m_cmdItemDataList.append(itemData);
        }
    } else {
        for (int i = 0; i < customCommandActionList.size(); i++) {
            QAction *curAction = customCommandActionList[i];
            QString strCmdName = curAction->text();
            QString strCmdText = curAction->data().toString();
            QKeySequence keySeq = curAction->shortcut();
            QString strKeySeq = keySeq.toString();
            if (strCmdName.contains(strFilter, Qt::CaseSensitivity::CaseInsensitive)
                    || strCmdText.contains(strFilter, Qt::CaseSensitivity::CaseInsensitive)
                    || strKeySeq.contains(strFilter, Qt::CaseSensitivity::CaseInsensitive)) {
                CustomCommandItemData itemData;
                itemData.m_cmdName = strCmdName;
                itemData.m_cmdText = strCmdText;
                itemData.m_cmdShortcut = strKeySeq;
                itemData.m_customCommandAction = curAction;

                m_cmdItemDataList.append(itemData);
            }
        }
    }

    m_cmdListModel->initCommandListData(m_cmdItemDataList);
}

void CustomCommandList::addNewCustomCommandData(QAction *actionData)
{
    if (actionData == nullptr) {
        return;
    }

    QAction *curAction = actionData;
    QString strCmdName = curAction->text();
    QString strCmdText = curAction->data().toString();
    QKeySequence keySeq = curAction->shortcut();
    QString strKeySeq = keySeq.toString();

    CustomCommandItemData itemData;
    itemData.m_cmdName = strCmdName;
    itemData.m_cmdText = strCmdText;
    itemData.m_cmdShortcut = strKeySeq;
    itemData.m_customCommandAction = curAction;

    m_cmdListModel->addNewCommandData(itemData);
}

void CustomCommandList::handleModifyCustomCommand(CustomCommandItemData itemData, QModelIndex modelIndex)
{
    QAction *curItemAction = itemData.m_customCommandAction;

    CustomCommandOptDlg dlg(CustomCommandOptDlg::CCT_MODIFY, curItemAction, this);
    if (dlg.exec() == QDialog::Accepted) {
        QAction &newAction = dlg.getCurCustomCmd();
        QString tmp11 = newAction.shortcut().toString(QKeySequence::NativeText);
        if (newAction.text() != curItemAction->text()) {
//            takeItem(getItemRow(item));
            ShortcutManager::instance()->delCustomCommand(curItemAction);
            QAction *existAction = ShortcutManager::instance()->checkActionIsExist(newAction);
            if (nullptr == existAction) {
                QAction *actionData = ShortcutManager::instance()->addCustomCommand(newAction);
                addNewCustomCommandData(actionData);
            } else {
                existAction->setData(newAction.data());
                existAction->setShortcut(newAction.shortcut());
                refreshOneRowCommandInfo(existAction);
                ShortcutManager::instance()->saveCustomCommandToConfig(existAction);
            }
            emit listItemCountChange();
        } else {
            curItemAction->setData(newAction.data());
            curItemAction->setShortcut(newAction.shortcut());
//            item->refreshCommandInfo(&newAction);
            ShortcutManager::instance()->saveCustomCommandToConfig(curItemAction);
        }
    } else {

        if (dlg.isDelCurCommand()) {
            OperationConfirmDlg dlg;
            dlg.setOperatTypeName(tr("delete opt"));
            dlg.setTipInfo(tr("Do you sure to delete the %1").arg(curItemAction->text()));
            dlg.exec();
            if (dlg.getConfirmResult() == QDialog::Accepted) {
                removeCommandItem(itemData, modelIndex);
                ShortcutManager::instance()->delCustomCommand(curItemAction);
                emit listItemCountChange();
            }
        }
    }
}

void CustomCommandList::removeCommandItem(CustomCommandItemData itemData, QModelIndex modelIndex)
{
    this->m_cmdListModel->removeRow(modelIndex.row(), modelIndex.parent());
    this->m_cmdItemDataList.removeAt(modelIndex.row());
    this->removeItem(modelIndex.row());
}

void CustomCommandList::refreshOneRowCommandInfo(QAction *action)
{
    for (int i = 0; i < count(); i++) {
//        CustomCommandItem *curItem = qobject_cast<CustomCommandItem *>(itemWidget(item(i)));
//        if (curItem->getCurCustomCommandAction() == action) {
//            curItem->refreshCommandInfo(action);
//        }
    }
}

int CustomCommandList::getItemRow(CustomCommandItemData itemData)
{
    for (int i = 0; i < count(); i++) {
        CustomCommandItemData data = m_cmdItemDataList.at(i);
        if (itemData.m_cmdName == data.m_cmdName
                && itemData.m_cmdText == data.m_cmdText
                && itemData.m_cmdShortcut == data.m_cmdShortcut) {
            return i;
        }
    }
    return -1;
}

QRect getModifyIconRect(QRect visualRect)
{
    int modifyIconSize = 30;
    return QRect(visualRect.right() - modifyIconSize - 10, visualRect.top() + (visualRect.height() - modifyIconSize) / 2, modifyIconSize, modifyIconSize);
}

void CustomCommandList::mouseMoveEvent(QMouseEvent *event)
{
    DListView::mouseMoveEvent(event);
}

void CustomCommandList::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_bLeftMouse = true;
    } else {
        m_bLeftMouse = false;
    }

    DListView::mousePressEvent(event);

    qDebug() << "start Press!!";
    if (m_cmdListModel && m_cmdListModel->rowCount(QModelIndex()) == 0) {
        return;
    }

    QPoint clickPoint = event->pos();

    QModelIndex modelIndex = indexAt(clickPoint);
    QRect rect = visualRect(modelIndex);

    if (!modelIndex.isValid()) {
        return;
    }

    CustomCommandItemData itemData =
        qvariant_cast<CustomCommandItemData>(m_cmdListModel->data(modelIndex, Qt::DisplayRole));

    if (getModifyIconRect(rect).contains(clickPoint)) {
        handleModifyCustomCommand(itemData, modelIndex);
    } else {
        qDebug() << "click item -- choose command";
        emit itemClicked(itemData, modelIndex);
    }
    m_cmdListModel->setData(modelIndex, QVariant::fromValue(itemData), Qt::DisplayRole);

}

void CustomCommandList::mouseReleaseEvent(QMouseEvent *event)
{
    DListView::mouseReleaseEvent(event);
}

void CustomCommandList::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{
    DListView::setSelection(rect, command);
}
