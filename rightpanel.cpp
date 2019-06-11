#include "rightpanel.h"

#include <QPropertyAnimation>

RightPanel::RightPanel(QWidget *parent)
    : QWidget(parent)
{
    // hide by default
    QWidget::hide();
}

void RightPanel::show()
{
    QWidget::show();
    QWidget::raise();

    setFocus();

    QRect rect = geometry();
    QRect windowRect = window()->geometry();
    QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry");
    animation->setDuration(250);
    animation->setEasingCurve(QEasingCurve::OutQuad);
    animation->setStartValue(QRect(windowRect.width(), rect.y(), rect.width(), rect.height()));
    animation->setEndValue(QRect(windowRect.width() - rect.width(), rect.y(), rect.width(), rect.height()));
    animation->start();

//    connect(animation, &QPropertyAnimation::valueChanged, this, [=] { m_themeView->adjustScrollbarMargins(); });
    connect(animation, &QPropertyAnimation::finished, animation, &QPropertyAnimation::deleteLater);
}

void RightPanel::hide()
{
    QRect rect = geometry();
    QRect windowRect = window()->geometry();
    QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry");
    animation->setDuration(250);
    animation->setEasingCurve(QEasingCurve::OutQuad);
    animation->setStartValue(QRect(windowRect.width() - rect.width(), rect.y(), rect.width(), rect.height()));
    animation->setEndValue(QRect(windowRect.width(), rect.y(), rect.width(), rect.height()));

    animation->start();

    connect(animation, &QPropertyAnimation::finished, this, &QWidget::hide);
    connect(animation, &QPropertyAnimation::finished, animation, &QPropertyAnimation::deleteLater);
}
