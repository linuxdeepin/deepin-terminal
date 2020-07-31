/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:zhangmeng@uniontech.com
 *
 * Maintainer:编码插件列表视图
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ENCODELISTVIEW_H
#define ENCODELISTVIEW_H

#include "mainwindow.h"
#include <DListView>
#include <DStyledItemDelegate>

DWIDGET_USE_NAMESPACE

class EncodeListModel;
class EncodeListView : public DListView
{
    Q_OBJECT

public:
    EncodeListView(QWidget *parent = nullptr);

protected:
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

    void resizeContents(int width, int height);
    QSize contentsSize() const;

signals:
    void focusOut();

public slots:
    void onListViewClicked(const QModelIndex &index);
    void checkEncode(QString encode);
    /** add by ut001121 zhangmeng 20200718 for sp3 keyboard interaction*/
    Qt::FocusReason getFocusReason(){return m_foucusReason;}

private:
    void initEncodeItems();

    MainWindow *m_Mainwindow  = nullptr;

    EncodeListModel *m_encodeModel = nullptr;

    QStandardItemModel *m_standardModel = nullptr;
    const int m_Space = 10;
    const int m_ContentWidth = 220;
    const int m_ContentHeight = 50;
    const int m_ListLenth = 1500;

    Qt::FocusReason m_foucusReason = Qt::NoFocusReason;
    QModelIndex m_modelIndexChecked;

};

class EncodeDelegate : public DStyledItemDelegate
{
public:
    explicit EncodeDelegate(QAbstractItemView *parent = nullptr)
        : DStyledItemDelegate(parent), m_parentView(parent){}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const Q_DECL_OVERRIDE;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const Q_DECL_OVERRIDE;

private:
    QAbstractItemView *m_parentView = nullptr;
};

#endif  // THEMELISTVIEW_H
