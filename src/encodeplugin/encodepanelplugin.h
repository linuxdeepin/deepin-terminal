#ifndef ENCODEPANELPLUGIN_H
#define ENCODEPANELPLUGIN_H

#include "mainwindowplugininterface.h"
#include "encodepanel.h"
#include "termwidget.h"
#include <QTextCodec>
//class EncodePanel;
class MainWindow;
class TermWidget;
class EncodePanelPlugin : public MainWindowPluginInterface
{
    Q_OBJECT
public:
    EncodePanelPlugin(QObject *parent);

    void initPlugin(MainWindow *mainWindow) override;
    QAction *titlebarMenu(MainWindow *mainWindow) override;

    EncodePanel *getEncodePanel();
    void initEncodePanel();
    // 设置当前term编码
    inline void setCurrentTermEncode(TermWidget *term);

private:
    MainWindow *m_mainWindow = nullptr;
    EncodePanel *m_encodePanel = nullptr;
};

#endif  // THEMEPANELPLUGIN_H
