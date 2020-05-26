#include "customcommandlist.h"
#include "customcommanddelegate.h"
#include "customcommandoptdlg.h"
#include "operationconfirmdlg.h"
#include "shortcutmanager.h"
#include "utils.h"
#include"service.h"

#include <DMessageBox>
#include <DLog>

#include <QAction>
#include<DScrollBar>
#include<DApplicationHelper>

CustomCommandList::CustomCommandList(bool &NotNeedRefresh, QWidget *parent) : DListView(parent), m_bNotNeedRefresh(NotNeedRefresh)
{
    setBackgroundRole(QPalette::NoRole);
    setAutoFillBackground(false);
    setUpdatesEnabled(true);

    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    initData();
}

CustomCommandList::~CustomCommandList()
{
    if (m_pdlg) {
        delete m_pdlg;
        m_pdlg = nullptr;
    }
}

void CustomCommandList::initData()
{
    m_cmdListModel = new QStandardItemModel;
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
    m_cmdListModel->clear();
    m_cmdItemDataList.clear();

    QList<QAction *> &customCommandActionList = ShortcutManager::instance()->getCustomCommandActionList();
    qDebug() << __FUNCTION__ << strFilter  << " : " << customCommandActionList.size();

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

    m_cmdProxyModel->addNewCommandData(itemData);
}

void CustomCommandList::handleModifyCustomCommand(CustomCommandItemData &itemData, QModelIndex modelIndex)
{
    window()->setEnabled(false);
    if (m_pdlg) {
        delete m_pdlg;
        m_pdlg = nullptr;
    }
    m_pdlg = new CustomCommandOptDlg(CustomCommandOptDlg::CCT_MODIFY, &itemData, this);
    m_pdlg->setModelIndex(modelIndex);
    connect(m_pdlg, &CustomCommandOptDlg::finished, this, [ &](int result) {
        window()->setEnabled(true);
        // 弹窗隐藏或消失
        Service::instance()->setIsDialogShow(window(), false);
        if (result == QDialog::Accepted) {
            QAction *newAction = m_pdlg->getCurCustomCmd();
            CustomCommandItemData itemData = *(m_pdlg->m_currItemData);
            CustomCommandItemData itemDel = itemData;

            itemData.m_cmdName = newAction->text();
            itemData.m_cmdText = newAction->data().toString();
            itemData.m_cmdShortcut = newAction->shortcut().toString();
            newAction->setData(newAction->data());
            newAction->setShortcut(newAction->shortcut());

            ShortcutManager::instance()->delCustomCommandForModify(itemDel);
            ShortcutManager::instance()->delCustomCommandForModify(itemData);
            addNewCustomCommandData(newAction);
            ShortcutManager::instance()->addCustomCommand(*newAction);
            removeCommandItem(m_pdlg->modelIndex);
            scrollToBottom();

            m_bNotNeedRefresh = true;
            m_pdlg->closeRefreshDataConnection();
            emit Service::instance()->refreshCommandPanel();

        } else {

            //Delete custom command
            if (m_pdlg->isDelCurCommand()) {
                DDialog *dlgDelete = new DDialog(this);

                //dlgDelete->setWindowModality(Qt::WindowModal);

                dlgDelete->setIcon(QIcon::fromTheme("deepin-terminal"));
                dlgDelete->setTitle(tr("Are you sure to delete?"));
                dlgDelete->addButton(QObject::tr("Cancel"), false, DDialog::ButtonNormal);
                dlgDelete->addButton(QObject::tr("OK"), false, DDialog::ButtonWarning);
                connect(dlgDelete, &DDialog::finished, this, [ & ](int result) {
                    if (result == QDialog::Accepted) {
                        CustomCommandItemData itemData = *(m_pdlg->m_currItemData);
                        ShortcutManager::instance()->delCustomCommand(*(m_pdlg->m_currItemData));
                        removeCommandItem(m_pdlg->modelIndex);
                        emit listItemCountChange();

                        m_bNotNeedRefresh = true;
                        m_pdlg->closeRefreshDataConnection();
                        emit Service::instance()->refreshCommandPanel();
                    }
                });
                dlgDelete->show();
            }
        }
    });
    m_pdlg->show();
    // 弹窗显示
    Service::instance()->setIsDialogShow(window(), true);
}

void CustomCommandList::removeCommandItem(QModelIndex modelIndex)
{
    Q_UNUSED(modelIndex)
    if (m_cmdListModel && m_cmdListModel->rowCount() == 0) {
        return;
    }

//    m_cmdProxyModel->removeRow(modelIndex.row(), modelIndex.parent());
    refreshCommandListData("");
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

    if (m_cmdListModel && m_cmdListModel->rowCount() == 0) {
        return;
    }

    if (!m_cmdProxyModel) {
        return;
    }

    QPoint clickPoint = event->pos();

    QModelIndex modelIndex = indexAt(clickPoint);
    QRect rect = visualRect(modelIndex);

    if (!modelIndex.isValid()) {
        return;
    }

    CustomCommandItemData itemData =
        qvariant_cast<CustomCommandItemData>(m_cmdProxyModel->data(modelIndex));

    if (getModifyIconRect(rect).contains(clickPoint)) {
        handleModifyCustomCommand(itemData, modelIndex);

    } else {
        emit itemClicked(itemData, modelIndex);
    }
}

void CustomCommandList::mouseReleaseEvent(QMouseEvent *event)
{
    DListView::mouseReleaseEvent(event);
}

void CustomCommandList::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{
    DListView::setSelection(rect, command);
}
