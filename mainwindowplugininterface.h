#ifndef MAINWINDOWPLUGININTERFACE_H
#define MAINWINDOWPLUGININTERFACE_H

#include <QAction>

class MainWindow;
class MainWindowPluginInterface : public QObject
{
    Q_OBJECT
public:
    MainWindowPluginInterface(QObject *parent) : QObject(parent) {}

    virtual void initPlugin(MainWindow *mainWindow) = 0;
    virtual QAction* titlebarMenu(MainWindow *mainWindow) = 0;
};

#endif // MAINWINDOWPLUGININTERFACE_H
