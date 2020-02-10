#ifndef SHORTCUTMANAGER_H
#define SHORTCUTMANAGER_H

#include "define.h"

#include <QList>
#include <QAction>

class MainWindow;
class ShortcutManager : public QObject
{
    Q_OBJECT
public:
    ShortcutManager(QObject *parent = nullptr);
    void setMainWindow(MainWindow *curMainWindow);
    static ShortcutManager *instance();
    void initShortcuts();

    QList<QAction *> createCustomCommandsFromConfig();
    QList<QAction *> createBuiltinShortcutsFromConfig();
    QList<QAction *> &getCustomCommandActionList();

    QAction *addCustomCommand(QAction &action);
    void delCustomCommand(QAction *action);
    void saveCustomCommandToConfig(QAction *action);
    void delCUstomCommandToConfig(QAction *action);
    // check same name of the action is exist
    QAction *checkActionIsExist(QAction &action);

private:
    MainWindow *m_mainWindow = nullptr;
    QList<QAction *> m_customCommandActionList;
    QList<QAction *> m_builtinShortcuts;
    static ShortcutManager *m_instance;
};

#endif  // SHORTCUTMANAGER_H
