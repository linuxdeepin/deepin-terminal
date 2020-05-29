#include "themepanelplugin.h"

#include "mainwindow.h"
#include "themepanel.h"
#include "termwidgetpage.h"
#include "settings.h"

ThemePanelPlugin::ThemePanelPlugin(QObject *parent) : MainWindowPluginInterface(parent)
{
    m_pluginName = "Theme";
}

void ThemePanelPlugin::initPlugin(MainWindow *mainWindow)
{
    m_mainWindow = mainWindow;
}

QAction *ThemePanelPlugin::titlebarMenu(MainWindow *mainWindow)
{
    QAction *switchThemeAction(new QAction("Switch theme", mainWindow));

    connect(switchThemeAction, &QAction::triggered, this, [this]() {
        getThemePanel()->show();
    });

    return switchThemeAction;
}

ThemePanel *ThemePanelPlugin::getThemePanel()
{
    if (!m_themePanel) {
        initThemePanel();
    }

    return m_themePanel;
}

void ThemePanelPlugin::initThemePanel()
{
    m_themePanel = new ThemePanel(m_mainWindow->centralWidget());

    connect(m_themePanel, &ThemePanel::themeChanged, this, [ = ](const QString themeName) {
        m_mainWindow->forAllTabPage([themeName](TermWidgetPage * tabPage) {
            tabPage->setColorScheme(themeName);
        });
        Settings::instance()->setColorScheme(themeName);
    });
}
