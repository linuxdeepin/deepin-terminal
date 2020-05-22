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
            /******** Add by nt001000 renfeixiang 2020-05-18:修改雷神窗口太小时，编码界面使用不方便，将雷神窗口变大适应正常的编码界面 Begin***************/
            if (m_mainWindow->isQuakeMode() && m_mainWindow->height() < 220) {
                m_mainWindow->resize(m_mainWindow->width(),220);//首先设置雷神界面的大小
                m_mainWindow->showPlugin(MainWindow::PLUGIN_TYPE_ENCODING);//重新打开编码界面，当前流程结束
                return;
            }
            /******** Add by nt001000 renfeixiang 2020-05-18:修改雷神窗口太小时，编码界面使用不方便，将雷神窗口变大适应正常的编码界面 End***************/
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
        mainWindow->showPlugin(MainWindow::PLUGIN_TYPE_ENCODING);
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
