#include "customcommandplugin.h"
#include "customcommandpanel.h"
#include "customcommandtoppanel.h"
#include "mainwindow.h"
#include "termwidgetpage.h"
#include "settings.h"
#include "shortcutmanager.h"

#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QLabel>

CustomCommandPlugin::CustomCommandPlugin(QObject *parent) : MainWindowPluginInterface(parent)
{
    m_pluginName = "Custom Command";
}

void CustomCommandPlugin::initPlugin(MainWindow *mainWindow)
{
    m_mainWindow = mainWindow;
    initCustomCommandTopPanel();
    connect(m_mainWindow, &MainWindow::showPluginChanged,  this, [ = ](const QString name) {
        if (MainWindow::PLUGIN_TYPE_CUSTOMCOMMAND != name) {
            getCustomCommandTopPanel()->hideAnim();
        } else {
            /******** Add by nt001000 renfeixiang 2020-05-18:修改雷神窗口太小时，自定义界面使用不方便，将雷神窗口变大适应正常的自定义界面 Begin***************/
            if (m_mainWindow->isQuakeMode() && m_mainWindow->height() < 220) {
                m_mainWindow->resize(m_mainWindow->width(), 220); //首先设置雷神界面的大小
                m_mainWindow->showPlugin(MainWindow::PLUGIN_TYPE_CUSTOMCOMMAND);//重新打开自定义界面，当前流程结束
                return;
            }
            /******** Add by nt001000 renfeixiang 2020-05-18:修改雷神窗口太小时，自定义界面使用不方便，将雷神窗口变大适应正常的自定义界面 End***************/
            getCustomCommandTopPanel()->show();
        }
    });
    connect(m_mainWindow, &MainWindow::quakeHidePlugin, this, [ = ]() {
        getCustomCommandTopPanel()->hide();
    });
}

QAction *CustomCommandPlugin::titlebarMenu(MainWindow *mainWindow)
{
    QAction *customCommandAction(new QAction(tr("Custom commands"), mainWindow));

    connect(customCommandAction, &QAction::triggered, mainWindow, [mainWindow]() {
        mainWindow->showPlugin(MainWindow::PLUGIN_TYPE_CUSTOMCOMMAND);
    });

    return customCommandAction;
}

void CustomCommandPlugin::initCustomCommandTopPanel()
{
    m_customCommandTopPanel = new CustomCommandTopPanel(m_mainWindow->centralWidget());
    connect(m_customCommandTopPanel,
            &CustomCommandTopPanel::handleCustomCurCommand,
            this,
            &CustomCommandPlugin::doCustomCommand);
}

CustomCommandTopPanel *CustomCommandPlugin::getCustomCommandTopPanel()
{
    if (!m_customCommandTopPanel) {
        initCustomCommandTopPanel();
    }

    return m_customCommandTopPanel;
}

void CustomCommandPlugin::doCustomCommand(const QString &strTxt)
{
    if (!strTxt.isEmpty()) {
        m_mainWindow->currentPage()->sendTextToCurrentTerm(strTxt);
        m_mainWindow->focusCurrentPage();
    }
    emit doHide();
}

void CustomCommandPlugin::hidePlugn()
{
    if (m_customCommandTopPanel && m_customCommandTopPanel->isVisible()) {
        m_customCommandTopPanel->hide();
    }
}
