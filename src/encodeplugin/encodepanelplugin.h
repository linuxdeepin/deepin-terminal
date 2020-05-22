#ifndef ENCODEPANELPLUGIN_H
#define ENCODEPANELPLUGIN_H

#include "mainwindowplugininterface.h"
#include "encodepanel.h"
#include <QTextCodec>
//class EncodePanel;
class MainWindow;
class EncodePanelPlugin : public MainWindowPluginInterface
{
    Q_OBJECT
public:
    EncodePanelPlugin(QObject *parent);

    void initPlugin(MainWindow *mainWindow) override;
    QAction *titlebarMenu(MainWindow *mainWindow) override;

    EncodePanel *getEncodePanel();
    void initEncodePanel();

private:
    MainWindow *m_mainWindow = nullptr;
    EncodePanel *m_encodePanel = nullptr;
};

#endif  // THEMEPANELPLUGIN_H
