// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ENCODELISTMODEL_H
#define ENCODELISTMODEL_H

#include <QStandardItemModel>

/*******************************************************************************
 1. @类名:    EncodeListModel
 2. @作者:    ut001121 zhangmeng
 3. @日期:    2020-08-11
 4. @说明:
*******************************************************************************/

class EncodeListModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit EncodeListModel(QObject *parent = nullptr);

    /**
     * @brief 获取行数
     * @author ut001121 zhangmeng
     * @param parent
     * @return
     */
    int rowCount(const QModelIndex &parent) const override;
    /**
     * @brief 根据行信息获取当前编码内容
     * @author ut001121 zhangmeng
     * @param index 行
     * @param role
     * @return
     */
    QVariant data(const QModelIndex &index, int role) const override;
    /**
     * @brief 返回编码数据列表
     * @author ut001121 zhangmeng
     * @return
     */
    QList<QByteArray> listData();

private:
    /**
     * @brief 初始化编码数据
     * @author ut001121 zhangmeng
     */
    void initEncodeData();

    QList<QByteArray> m_encodeData;
};

#endif  // THEMELISTMODEL_H
