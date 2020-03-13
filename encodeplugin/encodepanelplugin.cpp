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
}

QAction *EncodePanelPlugin::titlebarMenu(MainWindow *mainWindow)
{
    QAction *switchThemeAction(new QAction(tr("Switch &Encoding"), mainWindow));

    connect(switchThemeAction, &QAction::triggered, this, [this]() { getEncodePanel()->show(); });

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

    connect(m_encodePanel, &EncodePanel::encodeChanged, this, [=](const QByteArray encodeName) {
        m_mainWindow->forAllTabPage([encodeName](TermWidgetPage *tabPage) {
            qDebug() << "encodeName" << encodeName;
            tabPage->setTextCodec(QTextCodec::codecForName(encodeName));
            // tabPage->setColorScheme(encodeName);
        });
        // Settings::instance()->setColorScheme(encodeName);
    });
}
