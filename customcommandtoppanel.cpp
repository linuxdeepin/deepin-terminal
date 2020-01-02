#include "customcommandtoppanel.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <DPushButton>
#include <QDebug>
CustomCommandTopPanel::CustomCommandTopPanel(QWidget *parent) : RightPanel(parent)
    , m_customCommandPanel(new CustomCommandPanel(this))
    , m_customCommandSearchPanel(new CustomCommandSearchRstPanel(this))
{
    setAttribute(Qt::WA_TranslucentBackground);
    connect(m_customCommandPanel, &CustomCommandPanel::showSearchResult, this, &CustomCommandTopPanel::showCustomCommandSearchPanel);
    connect(m_customCommandPanel, &CustomCommandPanel::handleCustomCurCommand, this, &CustomCommandTopPanel::handleCustomCurCommand);
    connect(m_customCommandSearchPanel, &CustomCommandSearchRstPanel::showCustomCommandPanel, this, &CustomCommandTopPanel::showCustomCommandPanel);
    connect(m_customCommandSearchPanel, &CustomCommandSearchRstPanel::handleCustomCurCommand, this, &CustomCommandTopPanel::handleCustomCurCommand);
    connect(this, &CustomCommandTopPanel::focusOut, this, &RightPanel::hide);
}

void CustomCommandTopPanel::showCustomCommandPanel()
{
    qDebug() << "showCustomCommandPanel" << endl;
    m_customCommandPanel->refreshPanel();
    QPropertyAnimation *animation = new QPropertyAnimation(m_customCommandSearchPanel, "geometry");
    animation->setDuration(5000);
    animation->setEasingCurve(QEasingCurve::OutQuad);
    QRect rect = geometry();
    //QRect windowRect = m_customCommandSearchPanel->window()->geometry();
    animation->setStartValue(QRect(0, rect.y(), rect.width(), rect.height()));
    animation->setEndValue(QRect(rect.width(), rect.y(), rect.width(), rect.height()));
    connect(animation, &QPropertyAnimation::finished, m_customCommandSearchPanel, &QWidget::hide);
    connect(animation, &QPropertyAnimation::finished, animation, &QPropertyAnimation::deleteLater);

    QPropertyAnimation *animation1 = new QPropertyAnimation(m_customCommandPanel, "geometry");
    animation1->setDuration(5000);
    animation1->setEasingCurve(QEasingCurve::OutQuad);
    animation1->setStartValue(QRect(-rect.width(), rect.y(), rect.width(), rect.height()));
    animation1->setEndValue(QRect(0, rect.y(), rect.width(), rect.height()));
    connect(animation1, &QPropertyAnimation::finished, animation1, &QPropertyAnimation::deleteLater);
    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    group->addAnimation(animation);
    group->addAnimation(animation1);
    group->start();
}

void CustomCommandTopPanel::showCustomCommandSearchPanel(const QString &strFilter)
{
    qDebug() << "showCustomCommandSearchPanel" << endl;
    m_customCommandSearchPanel->refreshData(strFilter);
    m_customCommandSearchPanel->show();
    m_customCommandSearchPanel->setFocus();

    QPropertyAnimation *animation = new QPropertyAnimation(m_customCommandSearchPanel, "geometry");
    animation->setDuration(5000);
    animation->setEasingCurve(QEasingCurve::OutQuad);
    QRect rect = geometry();
    //QRect windowRect = m_customCommandSearchPanel->window()->geometry();
    animation->setStartValue(QRect(rect.width(), rect.y(), rect.width(), rect.height()));
    animation->setEndValue(QRect(0, rect.y(), rect.width(), rect.height()));
    connect(animation, &QPropertyAnimation::finished, animation, &QPropertyAnimation::deleteLater);

    QPropertyAnimation *animation1 = new QPropertyAnimation(m_customCommandPanel, "geometry");
    animation1->setDuration(5000);
    animation->setEasingCurve(QEasingCurve::OutQuad);
    animation1->setStartValue(QRect(0, rect.y(), rect.width(), rect.height()));
    animation1->setEndValue(QRect(- rect.width(), rect.y(), rect.width(), rect.height()));
    connect(animation1, &QPropertyAnimation::finished, m_customCommandPanel, &QWidget::hide);
    connect(animation1, &QPropertyAnimation::finished, animation1, &QPropertyAnimation::deleteLater);
    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    group->addAnimation(animation1);
    group->addAnimation(animation);
    group->start();
}

void CustomCommandTopPanel::show()
{
    RightPanel::show();
    m_customCommandPanel->resize(size());
    m_customCommandPanel->move(0, 0);
    m_customCommandPanel->show();
    m_customCommandPanel->refreshPanel();
    m_customCommandSearchPanel->resize(size());
    m_customCommandSearchPanel->hide();
}
