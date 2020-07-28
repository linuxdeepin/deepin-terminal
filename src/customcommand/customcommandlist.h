#ifndef CUSTOMCOMMANDLIST_H
#define CUSTOMCOMMANDLIST_H

#include "customcommanditemmodel.h"

#include <DListView>

#include <QWidget>
#include <QMouseEvent>
#include <QAction>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>

DWIDGET_USE_NAMESPACE

enum TabModifyType {
    TMT_NOTMOD,     //tab键盘控制，无修改操作
    TMT_MOD,        //tab键盘控制，修改操作
    TMT_DEL         //tab键盘控制，删除操作
};

class CustomCommandDelegate;
class CustomCommandItemModel;
class CustomCommandItem;
class CustomCommandOptDlg;
class CustomCommandList : public DListView
{
    Q_OBJECT
public:
    explicit CustomCommandList(QWidget *parent = nullptr);
    ~CustomCommandList();
    void refreshCommandListData(const QString &strFilter);
    void addNewCustomCommandData(QAction *actionData);
protected:
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command);

    void focusInEvent(QFocusEvent *event);
    void focusOutEvent(QFocusEvent *event);
    void keyPressEvent(QKeyEvent *event);

signals:
    void listItemCountChange();
    void itemClicked(CustomCommandItemData itemData, QModelIndex modelIndex);
    void resetTabModifyControlPositionSignal(unsigned int iTabModifyTime, TabModifyType tabModifyType);

private slots:
    void handleModifyCustomCommand(CustomCommandItemData &itemData, QModelIndex modelIndex);
    void resetTabModifyControlPositionSlot(unsigned int iTabModifyTime, TabModifyType tabModifyType);

public:
    bool m_bTabModify = false;
    int m_currentTabRow = 0;                // tab键盘控制时，列表项的行号
    bool m_bSearchRstPanelList = true;      // 是否是显示查找结果面板里的的list
    unsigned int  m_iTabModifyTime = 0;     //tab键盘操作的时间戳
    TabModifyType m_tabModType = TMT_NOTMOD;
    bool m_bforSearchPanel = false;

private:
    void initData();
    int getItemRow(CustomCommandItemData itemData);
    void removeCommandItem(QModelIndex modelIndex);

    CustomCommandDelegate *m_cmdDelegate = nullptr;
    QStandardItemModel *m_cmdListModel = nullptr;
    CustomCommandItemModel *m_cmdProxyModel = nullptr;
    QList<CustomCommandItemData> m_cmdItemDataList;
    bool m_bLeftMouse = true;
    CustomCommandOptDlg *m_pdlg = nullptr;
};

#endif  // CUSTOMCOMMANDLIST_H
