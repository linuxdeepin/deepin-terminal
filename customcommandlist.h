#ifndef CUSTOMCOMMANDLIST_H
#define CUSTOMCOMMANDLIST_H

#include <DListWidget>

#include <QWidget>
#include <QAction>

DWIDGET_USE_NAMESPACE

class CustomCommandItem;
class CustomCommandList : public DListWidget
{
    Q_OBJECT
public:
    explicit CustomCommandList(QWidget *parent = nullptr);
    void refreshData(const QString &strFilter);
    void addOneRowData(QAction *action);
    void refreshOneRowCommandInfo(QAction *action);

private:
    void initData();

signals:
    void listItemCountChange();

private slots:
    void handleModifyCustomCommand(CustomCommandItem *item);

private:
    int getItemRow(CustomCommandItem *findItem);
};

#endif  // CUSTOMCOMMANDLIST_H
