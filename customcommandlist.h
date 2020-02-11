#ifndef CUSTOMCOMMANDLIST_H
#define CUSTOMCOMMANDLIST_H

#include "customcommanditemmodel.h"

#include <DListView>

#include <QWidget>
#include <QAction>

DWIDGET_USE_NAMESPACE

class CustomCommandDelegate;
class CustomCommandItemModel;
class CustomCommandItem;
class CustomCommandList : public DListView
{
    Q_OBJECT
public:
    explicit CustomCommandList(QWidget *parent = nullptr);
    void refreshCommandListData(const QString &strFilter);
    void addNewCustomCommandData(QAction *actionData);
    void refreshOneRowCommandInfo(QAction *action);

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command);

private:
    void initData();

signals:
    void listItemCountChange();
    void itemClicked(CustomCommandItemData itemData, QModelIndex index);

private slots:
    void handleModifyCustomCommand(CustomCommandItemData itemData);

private:
    int getItemRow(CustomCommandItemData itemData);
    
    CustomCommandDelegate *m_cmdDelegate = nullptr;
    CustomCommandItemModel *m_cmdListModel = nullptr;
    bool m_bLeftMouse = true;
};

#endif  // CUSTOMCOMMANDLIST_H
