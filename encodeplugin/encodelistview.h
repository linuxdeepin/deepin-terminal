#ifndef ENCODELISTVIEW_H
#define ENCODELISTVIEW_H

#include <DListView>

DWIDGET_USE_NAMESPACE

class EncodeListView : public DListView
{
    Q_OBJECT

public:
    EncodeListView(QWidget *parent = nullptr);

protected:
    void focusOutEvent(QFocusEvent *event) override;
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;

signals:
    void focusOut();
};

#endif  // THEMELISTVIEW_H
