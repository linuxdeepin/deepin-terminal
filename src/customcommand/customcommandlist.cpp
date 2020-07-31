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

/*******************************************************************************
 1. @函数:    initData
 2. @作者:    sunchengxi
 3. @日期:    2020-07-31
 4. @说明:    初始化
*******************************************************************************/
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

    connect(this, SIGNAL(resetTabModifyControlPositionSignal(unsigned int, TabModifyType)), this, SLOT(resetTabModifyControlPositionSlot(unsigned int, TabModifyType)));
}

/*******************************************************************************
 1. @函数:    initData
 2. @作者:    sunchengxi
 3. @日期:    2020-07-31
 4. @说明:    刷新数据
*******************************************************************************/
void CustomCommandList::refreshCommandListData(const QString &strFilter)
{
    m_cmdListModel->clear();
    m_cmdItemDataList.clear();

    QList<QAction *> &customCommandActionList = ShortcutManager::instance()->getCustomCommandActionList();
    qDebug() << __FUNCTION__ << strFilter  << " : " << customCommandActionList.size();
    //根据条件进行刷新，strFilter 为空时 全部刷新，根据名称或者命令模糊匹配到strFilter的条件进行刷新
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

/*******************************************************************************
 1. @函数:    addNewCustomCommandData
 2. @作者:    sunchengxi
 3. @日期:    2020-07-31
 4. @说明:    列表中新增一条自定义命令项
*******************************************************************************/
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

/*******************************************************************************
 1. @函数:    handleModifyCustomCommand
 2. @作者:    sunchengxi
 3. @日期:    2020-07-31
 4. @说明:    修改自定义命令响应槽函数
*******************************************************************************/
void CustomCommandList::handleModifyCustomCommand(CustomCommandItemData &itemData, QModelIndex modelIndex)
{
    qDebug() <<  __FUNCTION__ << __LINE__;
    if (m_pdlg) {
        delete m_pdlg;
        m_pdlg = nullptr;
    }

    // 弹窗显示
    //Service::instance()->setIsDialogShow(window(), true); //暂时保留
    if (!m_bTabModify) {
        Service::instance()->setIsDialogShow(window(), true);
    }

    qDebug() << "modelIndex.row()=" << modelIndex.row() << ",this->count()=" << this->count();

    m_pdlg = new CustomCommandOptDlg(CustomCommandOptDlg::CCT_MODIFY, &itemData, this);
    m_pdlg->setModelIndex(modelIndex);
    connect(m_pdlg, &CustomCommandOptDlg::finished, this, [ &](int result) {

        int tempResult = result;
        m_pdlg->m_iTabModifyTime = this->m_iTabModifyTime;

        // 弹窗隐藏或消失
        //Service::instance()->setIsDialogShow(window(), false);
        if (!m_bTabModify) {
            Service::instance()->setIsDialogShow(window(), false);
        }

        if (QDialog::Accepted == result) {
            //确认修改处理
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
            addNewCustomCommandData(newAction);
            ShortcutManager::instance()->addCustomCommand(*newAction);
            removeCommandItem(m_pdlg->modelIndex);
            scrollToBottom();

            m_pdlg->closeRefreshDataConnection();
            emit Service::instance()->refreshCommandPanel(itemDel.m_cmdName, itemData.m_cmdName);

        } else if (QDialog::Rejected == result) {

            //Delete custom command 删除自定义命令处理
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
                    //是否删除自定义命令再次确认操作
                    if (QDialog::Accepted == result) {
                        CustomCommandItemData itemData = *(m_pdlg->m_currItemData);
                        ShortcutManager::instance()->delCustomCommand(*(m_pdlg->m_currItemData));
                        removeCommandItem(m_pdlg->modelIndex);
                        emit listItemCountChange();

                        m_pdlg->closeRefreshDataConnection();
                        emit Service::instance()->refreshCommandPanel(m_pdlg->m_currItemData->m_cmdName, m_pdlg->m_currItemData->m_cmdName);//emit Service::instance()->refreshCommandPanel("", "");
                    }

                    if (QDialog::Accepted == result) {
                        emit this->resetTabModifyControlPositionSignal(m_pdlg->m_iTabModifyTime, TMT_DEL);
                    } else {
                        emit this->resetTabModifyControlPositionSignal(m_pdlg->m_iTabModifyTime, TMT_NOTMOD);
                    }
                });
                dlgDelete->show();
            }
        }
        qDebug() << "================================tempResult=" << tempResult;
        if (!m_pdlg->isDelCurCommand()) {

            if (QDialog::Accepted == tempResult) {
                //修改操作，仅键盘操作时有效
                emit this->resetTabModifyControlPositionSignal(this->m_iTabModifyTime, TMT_MOD);
            } else {
                //未修改操作，仅键盘操作时有效
                emit this->resetTabModifyControlPositionSignal(this->m_iTabModifyTime, TMT_NOTMOD);
            }
        }

    });
    m_pdlg->show();
}

// 仅仅刷新自定义列表数据
void CustomCommandList::removeCommandItem(QModelIndex modelIndex)
{
    qDebug() <<  __FUNCTION__ << __LINE__;
    Q_UNUSED(modelIndex)
    if (m_cmdListModel && m_cmdListModel->rowCount() == 0) {
        return;
    }

    //    m_cmdProxyModel->removeRow(modelIndex.row(), modelIndex.parent()); //暂时保留
    refreshCommandListData("");
}

/*******************************************************************************
 1. @函数:    getItemRow
 2. @作者:    sunchengxi
 3. @日期:    2020-07-31
 4. @说明:    获取自定义列表数据的行id
*******************************************************************************/
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

/*******************************************************************************
 1. @函数:    getModifyIconRect
 2. @作者:    sunchengxi
 3. @日期:    2020-07-31
 4. @说明:    获取自定义列表项的编辑笔的区域
*******************************************************************************/
QRect getModifyIconRect(QRect visualRect)
{
    int modifyIconSize = 30;
    return QRect(visualRect.right() - modifyIconSize - 10, visualRect.top() + (visualRect.height() - modifyIconSize) / 2, modifyIconSize, modifyIconSize);
}

/*******************************************************************************
 1. @函数:    mouseMoveEvent
 2. @作者:    sunchengxi
 3. @日期:    2020-07-31
 4. @说明:    鼠标移动事件响应函数
*******************************************************************************/
void CustomCommandList::mouseMoveEvent(QMouseEvent *event)
{
    DListView::mouseMoveEvent(event);
}

/*******************************************************************************
 1. @函数:    mousePressEvent
 2. @作者:    sunchengxi
 3. @日期:    2020-07-31
 4. @说明:    鼠标按下事件响应函数
*******************************************************************************/
void CustomCommandList::mousePressEvent(QMouseEvent *event)
{
    m_cmdDelegate->m_bMouseOpt = false;//除了点击修改按钮和点击执行自定义命令，其他的都认为是无效的鼠标操作
    if (event->button() == Qt::LeftButton) {
        m_bLeftMouse = true;
    } else {
        m_bLeftMouse = false;
        return;
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

    m_bTabModify = false;
    m_cmdDelegate->m_bMouseOpt = true;

    CustomCommandItemData itemData = qvariant_cast<CustomCommandItemData>(m_cmdProxyModel->data(modelIndex));

    //鼠标点击自定义命令列表项，是进行修改操作还是执行自定义命令操作
    if (getModifyIconRect(rect).contains(clickPoint)) {
        handleModifyCustomCommand(itemData, modelIndex);

    } else {
        emit itemClicked(itemData, modelIndex);
    }
}

/*******************************************************************************
 1. @函数:    mouseReleaseEvent
 2. @作者:    sunchengxi
 3. @日期:    2020-07-31
 4. @说明:    鼠标按键弹起响应处理
*******************************************************************************/
void CustomCommandList::mouseReleaseEvent(QMouseEvent *event)
{
    DListView::mouseReleaseEvent(event);
}

/*******************************************************************************
 1. @函数:    setSelection
 2. @作者:    sunchengxi
 3. @日期:    2020-07-31
 4. @说明:    列表项数据选中设置
*******************************************************************************/
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
    qDebug() << "count=" << this->count() << ",currentIndex()=" << currentIndex().row() << ",m_currentTabRow=" << m_currentTabRow << ",event->reason():" << event->reason(); //-1

    if (event->reason() == Qt::TabFocusReason || event->reason() == Qt::BacktabFocusReason
            || event->reason() == Qt::OtherFocusReason) {
        m_cmdDelegate->m_bModifyCheck = false;
        m_cmdDelegate->m_bMouseOpt = false;

    }
    //列表有记录，在第一次tab键获得焦点，默认选中第一个记录项
    if (m_cmdItemDataList.size()) {
        QModelIndex qindex;
        if (event->reason() == Qt::ActiveWindowFocusReason || event->reason() == Qt::OtherFocusReason) {
            //自定义命令列表，非tab键盘操作获得焦点
            if (m_currentTabRow >= this->count()) {
                m_currentTabRow = this->count() - 1;
            }
            qindex = m_cmdProxyModel->index(m_currentTabRow, 0);
            qDebug() << "------------m_currentTabRow=" << m_currentTabRow;
        } else {
            //自定义命令列表，tab键盘操作获得焦点
            qindex = m_cmdProxyModel->index(0, 0);
        }
        setCurrentIndex(qindex);
        m_bforSearchPanel = true;

    } else {
        //focusNextPrevChild(!m_bSearchRstPanelList);//在列表数据被清空时，插件栏的选中框选中的位置，显示界面是，“添加”按钮，查找界面是“返回”按钮
        if (m_bSearchRstPanelList) {
            if (m_bforSearchPanel) {
                // 焦点落到 返回按钮
                QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_Tab, Qt::ShiftModifier);
                QApplication::sendEvent(Utils::getMainWindow(this), &keyPress);
                m_bforSearchPanel = false;
            } else {
                // tab 进入 +
                QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_Tab, Qt::MetaModifier);
                QApplication::sendEvent(Utils::getMainWindow(this), &keyPress);
            }

        } else {
            focusNextPrevChild(!m_bSearchRstPanelList);//自定义命令显示面板中的增加自定义命令按钮获取焦点
        }
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
            //排除鼠标点击列表空白项的干扰
            if (!m_cmdDelegate->m_bMouseOpt) {
                DBusManager::callSystemSound();
            }
        }

        if (m_cmdDelegate->m_bModifyCheck != false) {
            bNeedUpdate = true;
        }
        m_cmdDelegate->m_bModifyCheck = false;

        m_currentTabRow = this->currentIndex().row();
        break;
    }
    case Qt::Key_Right:
        if (m_cmdDelegate->m_bModifyCheck != true) {
            bNeedUpdate = true;
        }
        m_currentTabRow = this->currentIndex().row();
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
            m_currentTabRow = this->currentIndex().row();
            QDateTime time = QDateTime::currentDateTime();   //获取当前时间
            m_iTabModifyTime = time.toTime_t();   //将当前时间转为时间戳
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

void CustomCommandList::resetTabModifyControlPositionSlot(unsigned int iTabModifyTime, TabModifyType tabModifyType)
{
    static int i = 0;
    if (m_iTabModifyTime && (m_iTabModifyTime == iTabModifyTime)) {
        qDebug() << "resetTabModifyControlPositionSlot---" << i++ << ",time=" << iTabModifyTime << ",~" << tabModifyType << ",listCount=" << this->count();
    } else {
        return;
    }

    //未进行修改操作处理
    if (tabModifyType == TMT_NOTMOD && m_iTabModifyTime) {
        m_iTabModifyTime = 0;

        if (this->count()) {
            qDebug() << "m_currentTabRow-----------" << m_currentTabRow;
            QTimer::singleShot(30, this, [&]() {
                QModelIndex qindex = m_cmdProxyModel->index(m_currentTabRow, 0);
                setCurrentIndex(qindex);
                update();
            });
        }
    }
    //进行修改操作处理
    if (tabModifyType == TMT_MOD && m_iTabModifyTime) {
        m_iTabModifyTime = 0;
        if (this->count()) {
            qDebug() << "modify -----------";
            QTimer::singleShot(30, this, [&]() {
                QModelIndex qindex = m_cmdProxyModel->index(this->count() - 1, 0);
                m_currentTabRow = this->count() - 1;
                setCurrentIndex(qindex);
                update();
            });
        }
    }
    //进行删除操作处理
    if (tabModifyType == TMT_DEL && m_iTabModifyTime) {
        m_iTabModifyTime = 0;
        if (this->count()) {
            qDebug() << "modify -----------";
            QTimer::singleShot(30, this, [&]() {

                int iIndex = 0;
                if (m_currentTabRow >= this->count() - 1) {
                    iIndex = this->count() - 1;
                } else {
                    iIndex = m_currentTabRow;
                }
                if (0 == this->count()) {
                    return;
                }
                QModelIndex qindex = m_cmdProxyModel->index(iIndex, 0);
                m_currentTabRow = iIndex;
                setCurrentIndex(qindex);
                update();
            });
        }
    }
}


