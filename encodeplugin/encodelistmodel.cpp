#include "encodelistmodel.h"

#include "qtermwidget/lib/qtermwidget.h"

#include <QTextCodec>
#include <QDebug>
#include <QRegExp>

EncodeListModel::EncodeListModel(QObject *parent) : QAbstractListModel(parent)
{
    initEncodeData();
}

int EncodeListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    //
    return m_encodeData.count();
}

QVariant EncodeListModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(role);
    const int row = index.row();
    return m_encodeData[row];
}

void EncodeListModel::initEncodeData()
{
    QList<QByteArray> all = QTextCodec::availableCodecs();
    // m_encodeData = QTextCodec::availableCodecs().toSet().toList();
    // 这个过滤规则有待改进
    QMap<QByteArray, int> filter;
    for (QByteArray name : all)
    {
        QString strname = name;
        if (strname.contains(QRegExp("^[ibm]", Qt::CaseInsensitive))
            || strname.contains(QRegExp("^[windows]", Qt::CaseInsensitive))
            || strname.contains(QRegExp("^[cp]", Qt::CaseInsensitive)) || strname.contains(QRegExp("^\\d+_*\\d+$")))
        {
            continue;
        }
        if (filter.contains(name))
        {
            filter[name] = 1;
        }
        else
        {
            filter[name]++;
        }
    }
    m_encodeData = filter.keys();
    qDebug() << "QTextCodec::availableCodecs" << m_encodeData.count();
}
