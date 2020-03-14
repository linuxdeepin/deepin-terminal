#ifndef MYICONBUTTON_H
#define MYICONBUTTON_H

#include <DIconButton>

#include <QWidget>

DWIDGET_USE_NAMESPACE

class MyIconButton : public DIconButton
{
    Q_OBJECT
public:
    explicit MyIconButton(QWidget *parent = nullptr);

signals:
    void sigHoverd();

protected:
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
};

#endif  // MYICONBUTTON_H
