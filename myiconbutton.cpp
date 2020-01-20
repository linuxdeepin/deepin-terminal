#include "myiconbutton.h"
#include <QDebug>
MyIconButton::MyIconButton(QWidget *parent) : DIconButton(parent) {}

void MyIconButton::enterEvent(QEvent *event)
{
    //    setIcon(QStyle::StandardPixmap::SP_DialogYesButton);
    setIcon(QIcon(":/images/icon/hover/edit_press.svg"));
    DIconButton::enterEvent(event);
}

void MyIconButton::leaveEvent(QEvent *event)
{
    //    setIcon(QStyle::StandardPixmap::SP_DialogSaveButton);
    setIcon(QIcon(":/images/icon/hover/edit_hover.svg"));
    DIconButton::leaveEvent(event);
}
