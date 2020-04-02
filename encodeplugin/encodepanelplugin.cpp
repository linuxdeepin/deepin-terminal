#include "encodepanelplugin.h"

#include "mainwindow.h"
#include "encodepanel.h"
#include "termwidgetpage.h"
#include "settings.h"

#include <DLog>

EncodePanelPlugin::EncodePanelPlugin(QObject *parent) : MainWindowPluginInterface(parent)
{
    m_pluginName = "Encoding";
}

void EncodePanelPlugin::initPlugin(MainWindow *mainWindow)
{
    m_mainWindow = mainWindow;
    initEncodePanel();
    connect(m_mainWindow, &MainWindow::showPluginChanged,  this, [=](const QString name)
    {
        if(MainWindow::PLUGIN_TYPE_ENCODING != name)
        {
            getEncodePanel()->hideAnim();
        }
        else {
            getEncodePanel()->show();
        }
    });
    connect(m_mainWindow, &MainWindow::quakeHidePlugin, this, [ = ]() {
        getEncodePanel()->hide();
    });
}

QAction *EncodePanelPlugin::titlebarMenu(MainWindow *mainWindow)
{
    QAction *switchThemeAction(new QAction(tr("Switch &Encoding"), mainWindow));

    connect(switchThemeAction, &QAction::triggered, mainWindow, [mainWindow]() {
        emit mainWindow->showPluginChanged(MainWindow::PLUGIN_TYPE_ENCODING);
    });

    return switchThemeAction;
}

EncodePanel *EncodePanelPlugin::getEncodePanel()
{
    if (m_encodePanel == nullptr) {
        initEncodePanel();
    }

    return m_encodePanel;
}

void EncodePanelPlugin::initEncodePanel()
{
    m_encodePanel = new EncodePanel(m_mainWindow->centralWidget());
}
