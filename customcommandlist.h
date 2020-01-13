#ifndef CUSTOMCOMMANDLIST_H
#define CUSTOMCOMMANDLIST_H

#include <QWidget>
#include <DListWidget>
#include <QAction>
class CustomCommandItem;
DWIDGET_USE_NAMESPACE

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
protected:

private slots:
    void handleModifyCustomCommand(CustomCommandItem *item);
private:
    int getItemRow(CustomCommandItem *findItem);
};

#endif // CUSTOMCOMMANDLIST_H
