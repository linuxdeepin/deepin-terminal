#ifndef ENCODELISTVIEW_H
#define ENCODELISTVIEW_H

#include <DListView>

DWIDGET_USE_NAMESPACE

class EncodeListModel;
class EncodeListView : public DListView
{
    Q_OBJECT

public:
    EncodeListView(QWidget *parent = nullptr);

protected:
    void focusOutEvent(QFocusEvent *event) override;
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;
    void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command) override;

    void resizeContents(int width, int height);
    QSize contentsSize() const;

signals:
    void focusOut();

public slots:
    void onListViewClicked(const QModelIndex &index);

private:
    void initEncodeItems();

    EncodeListModel *m_encodeModel = nullptr;

    QStandardItemModel *m_standardModel = nullptr;
    const int m_Space = 10;
    const int m_ContentWidth = 220;
    const int m_ContentHeight = 50;
    const int m_BarWidth = 35;
    const int m_ListLenth = 1500;

};

#endif  // THEMELISTVIEW_H
