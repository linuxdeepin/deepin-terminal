#include "themepanel.h"

#include "themelistview.h"
#include "themelistmodel.h"
#include "themeitemdelegate.h"

#include <QDebug>
#include <QScroller>
#include <QVBoxLayout>
#include <QPropertyAnimation>

ThemePanel::ThemePanel(QWidget *parent)
    : QWidget(parent),
      m_themeView(new ThemeListView(this)),
      m_themeModel(new ThemeListModel(this))
{
    // init view.
    m_themeView->setModel(m_themeModel);
    m_themeView->setItemDelegate(new ThemeItemDelegate(this));

    setFocusProxy(m_themeView);
    QScroller::grabGesture(m_themeView, QScroller::TouchGesture);

    // init layout.
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_themeView);
    setStyleSheet("background-color: brown;");

    layout->setMargin(0);
    layout->setSpacing(0);
    setFixedWidth(250);

    QWidget::hide();

    connect(m_themeView, &ThemeListView::focusOut, this, &ThemePanel::hide);
}

void ThemePanel::show()
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
