#ifndef CUSTOMCOMMANDITEMMODEL_H
#define CUSTOMCOMMANDITEMMODEL_H

#include <QSortFilterProxyModel>
#include <QAction>

typedef struct {
    QString m_cmdName;
    QString m_cmdText;
    QString m_cmdShortcut;
    QAction *m_customCommandAction;
} CustomCommandItemData;

Q_DECLARE_METATYPE(CustomCommandItemData)

class CustomCommandItemModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    CustomCommandItemModel(QObject *parent = nullptr);

    void initCommandListData(const QList<CustomCommandItemData> &cmdListData);
    void addNewCommandData(const CustomCommandItemData itemData);
};

#endif // CUSTOMCOMMANDITEMMODEL_H
