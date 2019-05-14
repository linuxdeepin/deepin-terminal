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

signals:
    void focusOut();
};

#endif // THEMELISTVIEW_H
