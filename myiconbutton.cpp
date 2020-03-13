#include "myiconbutton.h"

#include <DLog>

MyIconButton::MyIconButton(QWidget *parent) : DIconButton(parent)
{
}

void MyIconButton::enterEvent(QEvent *event)
{
    setIcon(QIcon(":/images/icon/focus/edit_press.svg"));
    DIconButton::enterEvent(event);
}

void MyIconButton::leaveEvent(QEvent *event)
{
    setIcon(QIcon(":/images/icon/hover/edit_hover.svg"));
    DIconButton::leaveEvent(event);
}
