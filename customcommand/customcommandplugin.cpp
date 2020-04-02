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
    connect(m_mainWindow, &MainWindow::showPluginChanged,  this, [=](const QString name)
    {
        if(MainWindow::PLUGIN_TYPE_CUSTOMCOMMAND != name)
        {
            getCustomCommandTopPanel()->hideAnim();
        }
        else {
            getCustomCommandTopPanel()->show();
        }
    });
    connect(m_mainWindow, &MainWindow::quakeHidePlugin, this, [ = ]() {
        getCustomCommandTopPanel()->hide();
    });
}

QAction *CustomCommandPlugin::titlebarMenu(MainWindow *mainWindow)
{
    QAction *customCommandAction(new QAction(tr("Custom Commands"), mainWindow));

    connect(customCommandAction, &QAction::triggered, mainWindow, [mainWindow]() {
        emit mainWindow->showPluginChanged(MainWindow::PLUGIN_TYPE_CUSTOMCOMMAND);
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
        m_mainWindow->currentTab()->sendTextToCurrentTerm(strTxt);
    }
    emit doHide();
}

void CustomCommandPlugin::hidePlugn()
{
    if (m_customCommandTopPanel && m_customCommandTopPanel->isVisible()) {
        m_customCommandTopPanel->hide();
    }
}
