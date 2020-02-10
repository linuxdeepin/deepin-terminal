#ifndef CUSTOMCOMMANDITEMMODEL_H
#define CUSTOMCOMMANDITEMMODEL_H

#include <QStandardItemModel>
#include <QAction>

typedef struct {
    QString m_cmdName;
    QString m_cmdText;
    QString m_cmdShortcut;
    QAction *m_customCommandAction;
}CustomCommandItemData;

Q_DECLARE_METATYPE(CustomCommandItemData)

class CustomCommandItemModel : public QStandardItemModel
{
    Q_OBJECT
public:
    CustomCommandItemModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    
    void initCommandListData(const QList<CustomCommandItemData> &cmdListData);
    void addNewCommandData(const CustomCommandItemData itemData);

private:

    QList<CustomCommandItemData> m_cmdListData;
};

#endif // CUSTOMCOMMANDITEMMODEL_H
