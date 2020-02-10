#ifndef CUSTOMCOMMANDLIST_H
#define CUSTOMCOMMANDLIST_H

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

private:
    void initData();

signals:
    void listItemCountChange();

private slots:
    void handleModifyCustomCommand(CustomCommandItem *item);

private:
    int getItemRow(CustomCommandItem *findItem);
    
    CustomCommandDelegate *m_cmdDelegate = nullptr;
    CustomCommandItemModel *m_cmdListModel = nullptr;
};

#endif  // CUSTOMCOMMANDLIST_H
