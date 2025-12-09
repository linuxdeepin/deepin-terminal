#include "switchthememenu.h"

SwitchThemeMenu::SwitchThemeMenu(const QString &title, QWidget *parent): QMenu(title, parent)
{
}

void SwitchThemeMenu::leaveEvent(QEvent *)
{
    //鼠标停靠悬浮判断
    bool ishover = this->property("hover").toBool();
    if (!ishover)
        emit mainWindowCheckThemeItemSignal();
}

void SwitchThemeMenu::hideEvent(QHideEvent *)
{
    hoveredThemeStr = "";
    emit menuHideSetThemeSignal();
}

void SwitchThemeMenu::enterEvent(QEvent *event)
{
    hoveredThemeStr = "";
    return QMenu::enterEvent(event);
}

void SwitchThemeMenu::keyPressEvent(QKeyEvent *event)
{
    //fix bug#64969主题中点击tab键不可以切换主题
    //内置主题屏蔽 除了 上下左右回车键的其他按键响应 处理bug#53439
    if (event->key() != Qt::Key_Space) {
        emit mainWindowCheckThemeItemSignal();
        return QMenu::keyPressEvent(event);
    }
}
