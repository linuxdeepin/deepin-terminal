#include "customcommandtoppanel.h"
#include "service.h"

#include <DPushButton>
#include <DLog>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

const int iAnimationDuration = 300;

CustomCommandTopPanel::CustomCommandTopPanel(QWidget *parent)
    : RightPanel(parent),
      m_customCommandPanel(new CustomCommandPanel(m_bNotNeedRefresh, this)),
      m_customCommandSearchPanel(new CustomCommandSearchRstPanel(m_bNotNeedRefresh, this))
{
    setAttribute(Qt::WA_TranslucentBackground);
    connect(m_customCommandPanel,
            &CustomCommandPanel::showSearchResult,
            this,
            &CustomCommandTopPanel::showCustomCommandSearchPanel);
    connect(m_customCommandPanel,
            &CustomCommandPanel::handleCustomCurCommand,
            this,
            &CustomCommandTopPanel::handleCustomCurCommand);
    connect(m_customCommandSearchPanel,
            &CustomCommandSearchRstPanel::showCustomCommandPanel,
            this,
            &CustomCommandTopPanel::showCustomCommandPanel);
    connect(m_customCommandSearchPanel,
            &CustomCommandSearchRstPanel::handleCustomCurCommand,
            this,
            &CustomCommandTopPanel::handleCustomCurCommand);
    connect(this, &CustomCommandTopPanel::handleCustomCurCommand, this, &RightPanel::hideAnim);

    connect(Service::instance(), &Service::refreshCommandPanel, this, &CustomCommandTopPanel::slotsRefreshCommandPanel);
}

void CustomCommandTopPanel::showCustomCommandPanel()
{
    qDebug() << "showCustomCommandPanel" << endl;
    m_customCommandPanel->resize(size());
    m_customCommandPanel->refreshCmdPanel();
    m_customCommandPanel->show();

    QPropertyAnimation *animation = new QPropertyAnimation(m_customCommandSearchPanel, "geometry");
    animation->setDuration(iAnimationDuration);
    animation->setEasingCurve(QEasingCurve::OutQuad);

    QRect rect = geometry();
    animation->setStartValue(QRect(0, rect.y(), rect.width(), rect.height()));
    animation->setEndValue(QRect(rect.width(), rect.y(), rect.width(), rect.height()));
    connect(animation, &QPropertyAnimation::finished, m_customCommandSearchPanel, &QWidget::hide);
    connect(animation, &QPropertyAnimation::finished, animation, &QPropertyAnimation::deleteLater);

    QPropertyAnimation *animation1 = new QPropertyAnimation(m_customCommandPanel, "geometry");
    animation1->setDuration(iAnimationDuration);
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
    animation->setDuration(iAnimationDuration);
    animation->setEasingCurve(QEasingCurve::OutQuad);

    QRect rect = geometry();
    animation->setStartValue(QRect(rect.width(), rect.y(), rect.width(), rect.height()));
    animation->setEndValue(QRect(0, rect.y(), rect.width(), rect.height()));
    connect(animation, &QPropertyAnimation::finished, animation, &QPropertyAnimation::deleteLater);

    QPropertyAnimation *animation1 = new QPropertyAnimation(m_customCommandPanel, "geometry");
    animation1->setDuration(iAnimationDuration);
    animation->setEasingCurve(QEasingCurve::OutQuad);
    animation1->setStartValue(QRect(0, rect.y(), rect.width(), rect.height()));
    animation1->setEndValue(QRect(-rect.width(), rect.y(), rect.width(), rect.height()));
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
    m_customCommandPanel->refreshCmdPanel();
    m_customCommandSearchPanel->resize(size());
    m_customCommandSearchPanel->hide();
}

/******** Modify by nt001000 renfeixiang 2020-05-15:修改自定义界面，在Alt+F2时，隐藏在显示，高度变大问题 Begin***************/
//void CustomCommandTopPanel::resizeEvent(QResizeEvent *event)
//{
//    m_customCommandPanel->resize(size());
//}
/******** Modify by nt001000 renfeixiang 2020-05-15:修改自定义界面，在Alt+F2时，隐藏在显示，高度变大问题 End***************/

void CustomCommandTopPanel::slotsRefreshCommandPanel()
{
    if (m_bNotNeedRefresh) {
        m_bNotNeedRefresh = false;
        return;
    }
    m_customCommandPanel->resize(size());
    m_customCommandPanel->show();
    m_customCommandPanel->refreshCmdPanel();



}

