#ifndef THEMELISTMODEL_H
#define THEMELISTMODEL_H

#include <QAbstractListModel>

class ThemeListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    ThemeListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

private:
    void initThemeData();

    QList<QString> m_themeData;
};

#endif  // THEMELISTMODEL_H
