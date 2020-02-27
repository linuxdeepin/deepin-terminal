#include "customcommandlist.h"
#include "customcommanddelegate.h"
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

    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    initData();
}

void CustomCommandList::initData()
{
    m_cmdListModel = new QStandardItemModel();
    m_cmdProxyModel = new CustomCommandItemModel(this);
    m_cmdProxyModel->setSourceModel(m_cmdListModel);
    m_cmdProxyModel->setFilterRole(Qt::UserRole);
    m_cmdProxyModel->setDynamicSortFilter(true);

    m_cmdDelegate = new CustomCommandDelegate(this);
    this->setModel(m_cmdProxyModel);
    this->setItemDelegate(m_cmdDelegate);
}

void CustomCommandList::refreshCommandListData(const QString &strFilter)
{
    qDebug() << __FUNCTION__ << strFilter;
    m_cmdListModel->clear();
    m_cmdItemDataList.clear();

    QList<QAction *> &customCommandActionList = ShortcutManager::instance()->getCustomCommandActionList();

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

    m_cmdProxyModel->initCommandListData(m_cmdItemDataList);
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

    m_cmdProxyModel->addNewCommandData(itemData);
}

void CustomCommandList::handleModifyCustomCommand(CustomCommandItemData itemData, QModelIndex modelIndex)
{
    QAction *curItemAction = itemData.m_customCommandAction;
    qDebug() << "old name:" << itemData.m_cmdName;
    qDebug() << "old cmd:" << itemData.m_cmdText;
    qDebug() << "old shortcut:" << itemData.m_cmdShortcut;
    qDebug() << "old action:" << itemData.m_customCommandAction;

    CustomCommandOptDlg dlg(CustomCommandOptDlg::CCT_MODIFY, curItemAction, this);
    if (dlg.exec() == QDialog::Accepted) {
        QAction *newAction = dlg.getCurCustomCmd();
        QString strActionShortcut = newAction->shortcut().toString(QKeySequence::NativeText);

        qDebug() << "strActionShortcut:" << strActionShortcut;
        qDebug() << "newAction:" << newAction;
        itemData.m_cmdName = newAction->text();
        itemData.m_cmdText = newAction->data().toString();
        itemData.m_cmdShortcut = newAction->shortcut().toString();
        newAction->setData(newAction->data());
        newAction->setShortcut(newAction->shortcut());
        itemData.m_customCommandAction = newAction;

        ShortcutManager::instance()->delCUstomCommandToConfig(curItemAction);
        ShortcutManager::instance()->saveCustomCommandToConfig(itemData.m_customCommandAction);

        m_cmdProxyModel->setData(modelIndex, QVariant::fromValue(itemData));
    } else {

        //Delete custom command
        if (dlg.isDelCurCommand()) {
            OperationConfirmDlg dlg;
            dlg.setOperatTypeName(tr("delete opt"));
            dlg.setTipInfo(tr("Do you sure to delete the %1").arg(curItemAction->text()));
            dlg.exec();
            if (dlg.getConfirmResult() == QDialog::Accepted) {
                removeCommandItem(modelIndex);
                ShortcutManager::instance()->delCustomCommand(curItemAction);
                emit listItemCountChange();
            }
        }
    }
}

void CustomCommandList::removeCommandItem(QModelIndex modelIndex)
{
    m_cmdProxyModel->removeRow(modelIndex.row());
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

    if (m_cmdProxyModel && m_cmdProxyModel->rowCount() == 0) {
        return;
    }

    QPoint clickPoint = event->pos();

    QModelIndex modelIndex = indexAt(clickPoint);
    QRect rect = visualRect(modelIndex);

    if (!modelIndex.isValid()) {
        return;
    }

    CustomCommandItemData itemData =
        qvariant_cast<CustomCommandItemData>(m_cmdProxyModel->data(modelIndex, Qt::DisplayRole));

    if (getModifyIconRect(rect).contains(clickPoint)) {
        handleModifyCustomCommand(itemData, modelIndex);
    } else {
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
