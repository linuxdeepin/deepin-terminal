#include "customcommandlist.h"
#include "customcommanddelegate.h"
#include "customcommandoptdlg.h"
#include "operationconfirmdlg.h"
#include "shortcutmanager.h"
#include "utils.h"
#include"service.h"
#include "dbusmanager.h"

#include <DMessageBox>
#include <DLog>

#include <QAction>
#include<DScrollBar>
#include<DApplicationHelper>

CustomCommandList::CustomCommandList(QWidget *parent) : DListView(parent)
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
    m_cmdListModel = new QStandardItemModel(this);
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
    qDebug() <<  __FUNCTION__ << __LINE__;
    if (m_pdlg) {
        delete m_pdlg;
        m_pdlg = nullptr;
    }

    // 弹窗显示
    //Service::instance()->setIsDialogShow(window(), true);
    if (!m_bTabModify) {
        Service::instance()->setIsDialogShow(window(), true);
    }

    m_pdlg = new CustomCommandOptDlg(CustomCommandOptDlg::CCT_MODIFY, &itemData, this);
    m_pdlg->setModelIndex(modelIndex);
    connect(m_pdlg, &CustomCommandOptDlg::finished, this, [ &](int result) {
        // 弹窗隐藏或消失
        //Service::instance()->setIsDialogShow(window(), false);
        if (!m_bTabModify) {
            Service::instance()->setIsDialogShow(window(), false);
        }

        if (result == QDialog::Accepted) {
            qDebug() <<  __FUNCTION__ << __LINE__ << ":mod Custom Command";
            QAction *newAction = m_pdlg->getCurCustomCmd();
            CustomCommandItemData itemData = *(m_pdlg->m_currItemData);
            CustomCommandItemData itemDel = itemData;

            itemData.m_cmdName = newAction->text();
            itemData.m_cmdText = newAction->data().toString();
            itemData.m_cmdShortcut = newAction->shortcut().toString();
            newAction->setData(newAction->data());
            newAction->setShortcut(newAction->shortcut());
            ShortcutManager::instance()->delCustomCommand(itemDel);
            //ShortcutManager::instance()->delCustomCommandForModify(itemDel);
            //ShortcutManager::instance()->delCustomCommandForModify(itemData);
            addNewCustomCommandData(newAction);
            ShortcutManager::instance()->addCustomCommand(*newAction);
            removeCommandItem(m_pdlg->modelIndex);
            scrollToBottom();

            m_pdlg->closeRefreshDataConnection();
            emit Service::instance()->refreshCommandPanel(itemDel.m_cmdName, itemData.m_cmdName);

        } else if (result == QDialog::Rejected) {

            //Delete custom command
            if (m_pdlg->isDelCurCommand()) {
                qDebug() <<  __FUNCTION__ << __LINE__ << ":del Custom Command";
                DDialog *dlgDelete = new DDialog(this);
                dlgDelete->setAttribute(Qt::WA_DeleteOnClose);
                dlgDelete->setWindowModality(Qt::WindowModal);
                m_pdlg->m_dlgDelete = dlgDelete;

                dlgDelete->setIcon(QIcon::fromTheme("deepin-terminal"));
                dlgDelete->setTitle(tr("Are you sure you want to delete %1?").arg(m_pdlg->m_currItemData->m_cmdName));
                dlgDelete->addButton(QObject::tr("Cancel"), false, DDialog::ButtonNormal);
                dlgDelete->addButton(QObject::tr("Confirm"), true, DDialog::ButtonWarning);
                connect(dlgDelete, &DDialog::finished, this, [ & ](int result) {
                    if (result == QDialog::Accepted) {
                        CustomCommandItemData itemData = *(m_pdlg->m_currItemData);
                        ShortcutManager::instance()->delCustomCommand(*(m_pdlg->m_currItemData));
                        removeCommandItem(m_pdlg->modelIndex);
                        emit listItemCountChange();

                        m_pdlg->closeRefreshDataConnection();
                        emit Service::instance()->refreshCommandPanel(m_pdlg->m_currItemData->m_cmdName, m_pdlg->m_currItemData->m_cmdName);//emit Service::instance()->refreshCommandPanel("", "");
                    }
                });
                dlgDelete->show();
            }
        }
    });
    m_pdlg->show();
}

void CustomCommandList::removeCommandItem(QModelIndex modelIndex)
{
    qDebug() <<  __FUNCTION__ << __LINE__;
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
    m_cmdDelegate->m_bMouseOpt = true;
    m_cmdDelegate->m_iMouseOptRow = -1;
    if (event->button() == Qt::LeftButton) {
        m_bLeftMouse = true;
    } else {
        m_bLeftMouse = false;
    }

    DListView::mousePressEvent(event);

    if (false == m_bLeftMouse) {
        return;
    }

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

    m_cmdDelegate->m_iMouseOptRow = modelIndex.row();

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

/*******************************************************************************
 1. @函数:    focusInEvent
 2. @作者:    sunchengxi
 3. @日期:    2020-07-17
 4. @说明:    第一次获得焦点触发
*******************************************************************************/
void CustomCommandList::focusInEvent(QFocusEvent *event)
{
    qDebug() << "count=" << this->count() << ",currentIndex()=" << currentIndex().row() << ",event->reason():" << event->reason(); //-1

    //当不是tab键盘按下，比如方向键按下进来的，不处理 // 其实焦点已经过来了在index为-1处而已，需要其他操作 //tab键盘和 shift+tab键盘操作 好像天然 就是和方向键盘左右是通用的，在同级别的没有子控件的情况
//    if (Qt::TabFocusReason != event->reason()) {

//        return;
//    }
    if (event->reason() == Qt::TabFocusReason || event->reason() == Qt::BacktabFocusReason || event->reason() == Qt::ActiveWindowFocusReason) {
        m_cmdDelegate->m_bModifyCheck = false;
        m_cmdDelegate->m_bMouseOpt = false;

    }

    //列表有记录，在第一次tab键获得焦点，默认选中第一个记录项
    if (m_cmdItemDataList.size()) {
        QModelIndex qindex = m_cmdProxyModel->index(0, 0);
        setCurrentIndex(qindex);

    } else {
        focusNextPrevChild(!m_bSearchRstPanelList);//在列表数据被清空时，插件栏的选中框选中的位置，显示界面是，“添加”按钮，查找界面是“返回”按钮
    }

    if (m_bTabModify) {
        m_bTabModify = false;
    }

}

/*******************************************************************************
 1. @函数:    focusInEvent
 2. @作者:    sunchengxi
 3. @日期:    2020-07-17
 4. @说明:    焦点丢失触发
*******************************************************************************/
void CustomCommandList::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event);
    qDebug() <<  __FUNCTION__ << __LINE__;
    //tab键控制列表丢失焦点后，取消列表项的选中状态
    if (m_cmdItemDataList.size()) {
        QModelIndex qindex = m_cmdProxyModel->index(-1, 0);
        setCurrentIndex(qindex);

    }
    if (m_bTabModify) {
        setFocus();

    }

}

/*******************************************************************************
 1. @函数:    keyPressEvent
 2. @作者:    sunchengxi
 3. @日期:    2020-07-20
 4. @说明:    过滤键盘按下，方向键控制列表项操作
*******************************************************************************/
void CustomCommandList::keyPressEvent(QKeyEvent *event)
{
    static int is = 0;
    qDebug() << "CustomCommandList::keyPressEvent-" << is++;

    bool bNeedUpdate = false;
    switch (event->key()) {
    case Qt::Key_Up:
    case Qt::Key_Down:
    case Qt::Key_Left: {
        //上键到顶发出提示音 //下键到底发出提示音
        if ((Qt::Key_Up == event->key() && 0 == currentIndex().row()) || (Qt::Key_Down == event->key() && this->count() - 1 == currentIndex().row())) {
            DBusManager::callSystemSound();
        }

        if (m_cmdDelegate->m_bModifyCheck != false) {
            bNeedUpdate = true;
        }
        m_cmdDelegate->m_bModifyCheck = false;
        break;
    }
    case Qt::Key_Right:
        if (m_cmdDelegate->m_bModifyCheck != true) {
            bNeedUpdate = true;
        }
        m_cmdDelegate->m_bModifyCheck = true;
        break;
    case Qt::Key_Return:
    //case Qt::Key_Enter:
    case Qt::Key_Space: {
        qDebug() << "Key_Return   or  Key_Space  press";
        CustomCommandItemData itemData = qvariant_cast<CustomCommandItemData>(m_cmdProxyModel->data(this->currentIndex()));
        if (m_cmdDelegate->m_bModifyCheck) {
            m_bTabModify = true;
            m_cmdDelegate->m_bModifyCheck = false;
            m_currentRow = this->currentIndex().row();
            handleModifyCustomCommand(itemData, this->currentIndex());
        } else {
            emit itemClicked(itemData, this->currentIndex());
        }
        m_cmdDelegate->m_bMouseOpt = false;
        break;
    }

    default:
        break;
    }
    if (bNeedUpdate) {
        update();
    }
    return DListView::keyPressEvent(event);
}



