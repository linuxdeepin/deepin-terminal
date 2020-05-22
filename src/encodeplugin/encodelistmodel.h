#ifndef ENCODELISTMODEL_H
#define ENCODELISTMODEL_H

#include <QStandardItemModel>

class EncodeListModel : public QStandardItemModel
{
    Q_OBJECT
public:
    EncodeListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QList<QByteArray> listData();

private:
    void initEncodeData();

    QList<QByteArray> m_encodeData;
};

#endif  // THEMELISTMODEL_H
