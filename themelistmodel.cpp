#include "themelistmodel.h"

#include <qtermwidget5/qtermwidget.h>

ThemeListModel::ThemeListModel(QObject *parent) : QAbstractListModel(parent)
{
    initThemeData();
}

int ThemeListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_themeData.count();
}

QVariant ThemeListModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(role);
    const int row = index.row();
    return m_themeData[row];
}

void ThemeListModel::initThemeData()
{
    m_themeData = QTermWidget::availableColorSchemes();
}
