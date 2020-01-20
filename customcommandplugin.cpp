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
}

QAction *CustomCommandPlugin::titlebarMenu(MainWindow *mainWindow)
{
    QAction *customCommandAction(new QAction(tr("Custom Commands"), mainWindow));

    connect(customCommandAction, &QAction::triggered, this, [this]() { getCustomCommandTopPanel()->show(); });

    return customCommandAction;
}

void CustomCommandPlugin::initCustomCommandTopPanel()
{
    m_customCommandTopPanel = new CustomCommandTopPanel(m_mainWindow->centralWidget());
    connect(this, &CustomCommandPlugin::doHide, m_customCommandTopPanel, &CustomCommandTopPanel::focusOut);
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
