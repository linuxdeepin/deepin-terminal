#include "myiconbutton.h"
#include <QDebug>
MyIconButton::MyIconButton(QWidget *parent) : DIconButton(parent)
{

}

void MyIconButton::enterEvent(QEvent *event)
{
    setIcon(QStyle::StandardPixmap::SP_DialogYesButton);
    DIconButton::enterEvent(event);

}

void MyIconButton::leaveEvent(QEvent *event)
{
    setIcon(QStyle::StandardPixmap::SP_DialogSaveButton);
    DIconButton::leaveEvent(event);
}
