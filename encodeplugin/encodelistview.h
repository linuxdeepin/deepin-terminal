#ifndef ENCODELISTVIEW_H
#define ENCODELISTVIEW_H

#include <QListView>

class EncodeListView : public QListView
{
    Q_OBJECT

public:
    EncodeListView(QWidget *parent = nullptr);

protected:
    void focusOutEvent(QFocusEvent *event) override;
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;

signals:
    void focusOut();
    void encodeChanged(const QByteArray encodeName);
};

#endif  // THEMELISTVIEW_H
