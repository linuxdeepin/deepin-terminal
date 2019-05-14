#include "themepanel.h"

#include <QDebug>
#include <QScroller>
#include <QVBoxLayout>
#include <QPropertyAnimation>
#include <QLabel> // should remove

ThemePanel::ThemePanel(QWidget *parent)
    : QWidget(parent)
{
//    QScroller::grabGesture(m_themeView, QScroller::TouchGesture);
    // test
    QLabel* testWidget = new QLabel(" placeholder, should be removed");
    testWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(testWidget, &QWidget::customContextMenuRequested, this, [=](){hide();});

    // init layout.
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(testWidget);
    setStyleSheet("background-color: brown;");

    layout->setMargin(0);
    layout->setSpacing(0);
    setFixedWidth(250);

    QWidget::hide();
}

void ThemePanel::show()
{
    QWidget::show();
    QWidget::raise();
//    m_themeView->setFocus();

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

void ThemePanel::hide()
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
