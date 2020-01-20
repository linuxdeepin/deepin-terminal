#ifndef CUSTOMCOMMANDPLUGIN_H
#define CUSTOMCOMMANDPLUGIN_H

#include "customcommandtoppanel.h"
#include "mainwindowplugininterface.h"

#include <QObject>
#include <QWidget>

class CustomCommandSearchRstPanel;
class CustomCommandPanel;
class MainWindow;
class CustomCommandPlugin : public MainWindowPluginInterface
{
    Q_OBJECT
public:
    explicit CustomCommandPlugin(QObject *parent = nullptr);

    void initPlugin(MainWindow *mainWindow) override;
    QAction *titlebarMenu(MainWindow *mainWindow) override;

    CustomCommandTopPanel *getCustomCommandTopPanel();
    void initCustomCommandTopPanel();

signals:
    void doHide();

public slots:
    void doCustomCommand(const QString &strTxt);

private:
    MainWindow *m_mainWindow = nullptr;
    CustomCommandTopPanel *m_customCommandTopPanel = nullptr;
};

#endif  // CUSTOMCOMMANDPLUGIN_H
