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
      m_customCommandPanel(new CustomCommandPanel(this)),
      m_customCommandSearchPanel(new CustomCommandSearchRstPanel(this))
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
    /******** Modify by nt001000 renfeixiang 2020-05-28:修改将该行隐藏，RightPanel::hideAnim函数不会将自定义窗口标志设置未PLUGIN_TYPE_NONEbug#21992 Begin***************/
    //    connect(this, &CustomCommandTopPanel::handleCustomCurCommand, this, &RightPanel::hideAnim);
    /******** Modify by nt001000 renfeixiang 2020-05-28:修改将该行隐藏，RightPanel::hideAnim函数不会将自定义窗口标志设置未PLUGIN_TYPE_NONEbug#21992 Begin***************/
    m_customCommandPanel->hide();
    m_customCommandSearchPanel->hide();
    connect(Service::instance(), &Service::refreshCommandPanel, this, &CustomCommandTopPanel::slotsRefreshCommandPanel);
}

/*******************************************************************************
 1. @函数:    showCustomCommandPanel
 2. @作者:    sunchengxi
 3. @日期:    2020-08-05
 4. @说明:    显示自定义命令面板的槽函数,从自定义搜索面板切换到自定义命令面板
*******************************************************************************/
void CustomCommandTopPanel::showCustomCommandPanel()
{
    qDebug() << "showCustomCommandPanel" << endl;
    m_customCommandPanel->resize(size());
    m_customCommandPanel->refreshCmdPanel();
    m_customCommandPanel->show();
    m_customCommandPanel->m_isShow = true;
    m_customCommandSearchPanel->m_isShow = false;

    //从自定义搜索面板返回到自定义列表面板时，如果搜索面板存在焦点，则切换后光标焦点依然在自定义列表面板上
    MainWindow *main = Utils::getMainWindow(this);
    if (main != nullptr) {
        if (main->isFocusOnList()) {
            m_customCommandPanel->setFocusInPanel();
            qDebug() << "custom command panel has focus";
        }
    }

    QPropertyAnimation *animationCommandSearchPanel = new QPropertyAnimation(m_customCommandSearchPanel, "geometry");
    animationCommandSearchPanel->setDuration(iAnimationDuration);
    animationCommandSearchPanel->setEasingCurve(QEasingCurve::OutQuad);

    QRect rect = geometry();
    animationCommandSearchPanel->setStartValue(QRect(0, rect.y(), rect.width(), rect.height()));
    animationCommandSearchPanel->setEndValue(QRect(rect.width(), rect.y(), rect.width(), rect.height()));
    connect(animationCommandSearchPanel, &QPropertyAnimation::finished, m_customCommandSearchPanel, &QWidget::hide);
    connect(animationCommandSearchPanel, &QPropertyAnimation::finished, animationCommandSearchPanel, &QPropertyAnimation::deleteLater);

    QPropertyAnimation *animationCommandPanel = new QPropertyAnimation(m_customCommandPanel, "geometry");
    animationCommandPanel->setDuration(iAnimationDuration);
    animationCommandPanel->setEasingCurve(QEasingCurve::OutQuad);
    animationCommandPanel->setStartValue(QRect(-rect.width(), rect.y(), rect.width(), rect.height()));
    animationCommandPanel->setEndValue(QRect(0, rect.y(), rect.width(), rect.height()));
    connect(animationCommandPanel, &QPropertyAnimation::finished, animationCommandPanel, &QPropertyAnimation::deleteLater);

    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);
    group->addAnimation(animationCommandSearchPanel);
    group->addAnimation(animationCommandPanel);
    // 已验证：这个设定，会释放group以及所有组内动画。
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

/*******************************************************************************
 1. @函数:    showCustomCommandSearchPanel
 2. @作者:    sunchengxi
 3. @日期:    2020-08-05
 4. @说明:    根据搜索条件显示搜索面板的槽函数
*******************************************************************************/
void CustomCommandTopPanel::showCustomCommandSearchPanel(const QString &strFilter)
{
    qDebug() << "showCustomCommandSearchPanel" << endl;
    m_customCommandSearchPanel->refreshData(strFilter);
    m_customCommandSearchPanel->show();
    m_customCommandSearchPanel->m_backButton->setFocus();//m_customCommandSearchPanel->setFocus();

    QPropertyAnimation *animationCommandSearchPanel = new QPropertyAnimation(m_customCommandSearchPanel, "geometry");
    animationCommandSearchPanel->setDuration(iAnimationDuration);
    animationCommandSearchPanel->setEasingCurve(QEasingCurve::OutQuad);

    QRect rect = geometry();
    animationCommandSearchPanel->setStartValue(QRect(rect.width(), rect.y(), rect.width(), rect.height()));
    animationCommandSearchPanel->setEndValue(QRect(0, rect.y(), rect.width(), rect.height()));
    connect(animationCommandSearchPanel, &QPropertyAnimation::finished, animationCommandSearchPanel, &QPropertyAnimation::deleteLater);

    QPropertyAnimation *animationCommandPanel = new QPropertyAnimation(m_customCommandPanel, "geometry");
    animationCommandPanel->setDuration(iAnimationDuration);
    animationCommandSearchPanel->setEasingCurve(QEasingCurve::OutQuad);
    animationCommandPanel->setStartValue(QRect(0, rect.y(), rect.width(), rect.height()));
    animationCommandPanel->setEndValue(QRect(-rect.width(), rect.y(), rect.width(), rect.height()));
    connect(animationCommandPanel, &QPropertyAnimation::finished, m_customCommandPanel, &QWidget::hide);
    connect(animationCommandPanel, &QPropertyAnimation::finished, animationCommandPanel, &QPropertyAnimation::deleteLater);

    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);
    group->addAnimation(animationCommandPanel);
    group->addAnimation(animationCommandSearchPanel);
    // 已验证：这个设定，会释放group以及所有组内动画。
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

/*******************************************************************************
 1. @函数:    show
 2. @作者:    sunchengxi
 3. @日期:    2020-08-05
 4. @说明:    显示自定义列表面板，隐藏搜索面板，
             根据参数，控制自定义面板是否显示时是否带有焦点
*******************************************************************************/
void CustomCommandTopPanel::show(bool bSetFocus)
{
    RightPanel::show();
    m_customCommandPanel->resize(size());
    m_customCommandPanel->move(0, 0);
    m_customCommandPanel->show();
    m_customCommandPanel->refreshCmdPanel();
    m_customCommandSearchPanel->resize(size());
    m_customCommandSearchPanel->hide();
    m_bSetFocus = bSetFocus;
    if (m_bSetFocus) {
        // 将焦点设置在平面上
        m_customCommandPanel->setFocusInPanel();
    } else {
        // 将焦点落回终端
        Utils::getMainWindow(this)->focusCurrentPage();
    }

}

/******** Modify by nt001000 renfeixiang 2020-05-15:修改自定义界面，在Alt+F2时，隐藏在显示，高度变大问题 Begin***************/
//void CustomCommandTopPanel::resizeEvent(QResizeEvent *event)
//{
//    m_customCommandPanel->resize(size());
//}
/******** Modify by nt001000 renfeixiang 2020-05-15:修改自定义界面，在Alt+F2时，隐藏在显示，高度变大问题 End***************/

/*******************************************************************************
 1. @函数:    slotsRefreshCommandPanel
 2. @作者:    sunchengxi
 3. @日期:    2020-08-05
 4. @说明:    刷新自定义列表数据的槽函数
*******************************************************************************/
void CustomCommandTopPanel::slotsRefreshCommandPanel()
{
    m_customCommandPanel->resize(size());
    //m_customCommandPanel->show();
    m_customCommandPanel->refreshCmdPanel();
    m_customCommandSearchPanel->refreshData();

}


