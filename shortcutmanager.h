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
    ShortcutManager(MainWindow *parent);

    void initShortcuts();

    QList<QAction*> createCustomCommandsFromConfig();
    QList<QAction*> createBuiltinShortcutsFromConfig();

private:
    MainWindow *m_mainWindow = nullptr;

    QList<QAction*> m_customCommandList;
    QList<QAction*> m_builtinShortcuts;
};

#endif // SHORTCUTMANAGER_H
