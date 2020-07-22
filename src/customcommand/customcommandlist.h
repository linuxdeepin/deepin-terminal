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

private slots:
    void handleModifyCustomCommand(CustomCommandItemData &itemData, QModelIndex modelIndex);

public:
    bool m_bTabModify = false;
    int m_currentRow = 0;
    bool m_bSearchRstPanelList = true;// 是否是显示查找结果面板里的的list

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
