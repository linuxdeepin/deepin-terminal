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

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void initCommandListData(const QList<CustomCommandItemData> &cmdListData);
    void addNewCommandData(const CustomCommandItemData itemData);

private:

    QList<CustomCommandItemData> m_cmdListData;
};

#endif // CUSTOMCOMMANDITEMMODEL_H
