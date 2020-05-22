#ifndef THEMEPANELPLUGIN_H
#define THEMEPANELPLUGIN_H

#include "mainwindowplugininterface.h"

class ThemePanel;
class MainWindow;
class ThemePanelPlugin : public MainWindowPluginInterface
{
    Q_OBJECT
public:
    ThemePanelPlugin(QObject *parent);

    void initPlugin(MainWindow *mainWindow) override;
    QAction *titlebarMenu(MainWindow *mainWindow) override;

    ThemePanel *getThemePanel();
    void initThemePanel();

private:
    MainWindow *m_mainWindow = nullptr;
    ThemePanel *m_themePanel = nullptr;
};

#endif  // THEMEPANELPLUGIN_H
