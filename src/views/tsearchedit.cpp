#include "tsearchedit.h"
#include <QEvent>
#include <QKeyEvent>
#include <DWidget>

TSearchEdit::TSearchEdit(QWidget *parent)
    : DSearchEdit(parent)
{

}

bool TSearchEdit::eventFilter(QObject *target, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return) { //将event转换为QKeyEvent,然后判断是否键
            emit enter();
            return true;
        }
    }
    return DSearchEdit::eventFilter(target, event);
}
