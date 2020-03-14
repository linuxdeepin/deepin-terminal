#ifndef THEMELISTVIEW_H
#define THEMELISTVIEW_H

#include <QListView>

class ThemeListView : public QListView
{
    Q_OBJECT

public:
    ThemeListView(QWidget *parent = nullptr);

protected:
    void focusOutEvent(QFocusEvent *event) override;
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;

signals:
    void focusOut();
    void themeChanged(const QString themeName);
};

#endif  // THEMELISTVIEW_H
