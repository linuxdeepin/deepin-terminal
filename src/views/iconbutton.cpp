#include "iconbutton.h"

//qt
#include <QDebug>

IconButton::IconButton(QWidget *parent)
    : DIconButton(parent)
{

}

/*******************************************************************************
 1. @函数:    keyPressEvent
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-17
 4. @说明:    处理右键点击事件
*******************************************************************************/
void IconButton::keyPressEvent(QKeyEvent *event)
{
    // 不处理向右的事件
    switch (event->key()) {
    case Qt::Key_Right:
    case Qt::Key_Up:
    case Qt::Key_Down:
        event->ignore();
        break;
    case Qt::Key_Left:
        emit preFocus();
        break;
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Space:
        emit clicked(true);
        break;
    default:
        DIconButton::keyPressEvent(event);
        break;
    }
}

/*******************************************************************************
 1. @函数:    focusOutEvent
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-21
 4. @说明:    焦点出
*******************************************************************************/
void IconButton::focusOutEvent(QFocusEvent *event)
{
    qDebug() << event->reason() << "IconButton" << this;
    emit focusOut(event->reason());
    DIconButton::focusOutEvent(event);
}
