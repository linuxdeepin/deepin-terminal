#include "rightpanel.h"

#include <DAnchors>

#include <QPropertyAnimation>

DWIDGET_USE_NAMESPACE

RightPanel::RightPanel(QWidget *parent) : QWidget(parent)
{
    // hide by default.
    QWidget::hide();

    setFixedWidth(240 + 2);

    // Init theme panel.
    // 插件不支持resize,下面代码不需要了
//    DAnchorsBase::setAnchor(this, Qt::AnchorTop, parent, Qt::AnchorTop);
//    DAnchorsBase::setAnchor(this, Qt::AnchorBottom, parent, Qt::AnchorBottom);
//    DAnchorsBase::setAnchor(this, Qt::AnchorRight, parent, Qt::AnchorRight);
}

void RightPanel::show()
{
    QWidget::show();
    QWidget::raise();

    QRect rect = geometry();
    QRect windowRect = window()->geometry();
    QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry");
    animation->setDuration(250);
    animation->setEasingCurve(QEasingCurve::OutQuad);
    //animation->setStartValue(QRect(windowRect.width(), rect.y(), rect.width(), windowRect.height() - 50));
    //animation->setEndValue(QRect(windowRect.width() - rect.width(), rect.y(), rect.width(), windowRect.height() - 50));
    if (window()->isFullScreen()) {
        animation->setStartValue(QRect(windowRect.width(), rect.y(), rect.width(), windowRect.height()));
        animation->setEndValue(QRect(windowRect.width() - rect.width(), rect.y(), rect.width(), windowRect.height()));
    } else {
        animation->setStartValue(QRect(windowRect.width(), rect.y(), rect.width(), windowRect.height() - 50));
        animation->setEndValue(QRect(windowRect.width() - rect.width(), rect.y(), rect.width(), windowRect.height() - 50));
    }
    animation->start(QAbstractAnimation::DeleteWhenStopped);

    //    connect(animation, &QPropertyAnimation::valueChanged, this, [=] { m_themeView->adjustScrollbarMargins(); });
}

void RightPanel::hideAnim()
{
    // 隐藏状态不处理
    if (!isVisible()) {
        return;
    }
    QRect rect = geometry();
    QRect windowRect = window()->geometry();
    QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry");
    animation->setDuration(250);
    animation->setEasingCurve(QEasingCurve::OutQuad);
    //animation->setStartValue(
    //    QRect(windowRect.width() - rect.width(), rect.y(), rect.width(), windowRect.height() - 50));
    //animation->setEndValue(QRect(windowRect.width(), rect.y(), rect.width(), windowRect.height() - 50));
    if (window()->isFullScreen()) {
        animation->setStartValue(QRect(windowRect.width() - rect.width(), rect.y(), rect.width(), windowRect.height()));
        animation->setEndValue(QRect(windowRect.width(), rect.y(), rect.width(), windowRect.height()));
    } else {
        animation->setStartValue(QRect(windowRect.width() - rect.width(), rect.y(), rect.width(), windowRect.height() - 50));
        animation->setEndValue(QRect(windowRect.width(), rect.y(), rect.width(), windowRect.height() - 50));
    }

    /***mod begin by ut001121 zhangmeng 20200924 修复BUG49378***/
    //结束处理
    connect(animation, &QPropertyAnimation::finished, this, [=]{
        //启用面板
        setEnabled(true);
        //隐藏面板
        hide();
    });

    //禁用面板
    setEnabled(false);

    //开始动画
    animation->start(QAbstractAnimation::DeleteWhenStopped);
    /***mod end by ut001121***/
}
