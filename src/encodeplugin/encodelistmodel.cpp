// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "encodelistmodel.h"
#include "utils.h"

#include <DStandardItem>
#include <DLog>

DWIDGET_USE_NAMESPACE

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(encodeplugin,"org.deepin.terminal.encodeplugin")
#else
Q_LOGGING_CATEGORY(encodeplugin,"org.deepin.terminal.encodeplugin",QtInfoMsg)
#endif

EncodeListModel::EncodeListModel(QObject *parent) : QStandardItemModel(parent)
{
    qCDebug(encodeplugin) << "EncodeListModel constructor enter";
    Utils::set_Object_Name(this);
    initEncodeData();
    qCDebug(encodeplugin) << "EncodeListModel constructor exit";
}

int EncodeListModel::rowCount(const QModelIndex &parent) const
{
    qCDebug(encodeplugin) << "rowCount enter";
    Q_UNUSED(parent);
    //
    int count = m_encodeData.count();
    qCDebug(encodeplugin) << "rowCount exit, count:" << count;
    return count;
}

QVariant EncodeListModel::data(const QModelIndex &index, int role) const
{
    qCDebug(encodeplugin) << "data enter, row:" << index.row() << "role:" << role;
    Q_UNUSED(role);
    const int row = index.row();
    QVariant result = m_encodeData[row];
    qCDebug(encodeplugin) << "data exit, result:" << result;
    return result;
}

QList<QByteArray> EncodeListModel::listData()
{
    qCDebug(encodeplugin) << "listData enter";
    QList<QByteArray> result = m_encodeData;
    qCDebug(encodeplugin) << "listData exit, count:" << result.count();
    return result;
}

void EncodeListModel::initEncodeData()
{
    // 编码列表（含与系统编码表的大小写同步）统一由 Utils::encodeList() 维护
    m_encodeData = Utils::encodeList();

    qCInfo(encodeplugin) << "The number("<<  m_encodeData.count() << ") of system encoding formats supported by the terminal.";
}
