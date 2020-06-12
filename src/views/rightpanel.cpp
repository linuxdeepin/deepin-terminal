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
    DAnchorsBase::setAnchor(this, Qt::AnchorTop, parent, Qt::AnchorTop);
    DAnchorsBase::setAnchor(this, Qt::AnchorBottom, parent, Qt::AnchorBottom);
    DAnchorsBase::setAnchor(this, Qt::AnchorRight, parent, Qt::AnchorRight);
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
    animation->start();

    //    connect(animation, &QPropertyAnimation::valueChanged, this, [=] { m_themeView->adjustScrollbarMargins(); });
    connect(animation, &QPropertyAnimation::finished, animation, &QPropertyAnimation::deleteLater);
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


    animation->start();

    connect(animation, &QPropertyAnimation::finished, this, &QWidget::hide);
    connect(animation, &QPropertyAnimation::finished, animation, &QPropertyAnimation::deleteLater);
}
