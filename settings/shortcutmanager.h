#ifndef SHORTCUTMANAGER_H
#define SHORTCUTMANAGER_H

#include "define.h"
#include "customcommanditemmodel.h"
#include "utils.h"

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
    void mainWindowAddAction(QAction *action);
    void delCustomCommand(CustomCommandItemData itemData);
    void saveCustomCommandToConfig(QAction *action, int saveIndex);
    int delCustomCommandToConfig(CustomCommandItemData itemData);
    // check same name of the action is exist
    QAction *checkActionIsExist(QAction &action);
    void setClipboardCommandData(QString clipboardCommand);
    QString getClipboardCommandData();
    QString updateShortcut(const QString &Name, const QString &seq, bool loadMode = false);
    QString getShortcutSet(const QString &Name);
    bool    isShortcutExist(const QString &Key);
    bool    isValidShortcut(const QString &Key);

private:
    MainWindow *m_mainWindow = nullptr;
    QList<QAction *> m_customCommandActionList;
    //QList<QAction *> m_builtinShortcuts;
    QStringList m_builtinShortcuts;
    QMap<QString, QString> m_GloableShortctus;
    static ShortcutManager *m_instance;
    QString m_clipboardCommand;
};

#endif  // SHORTCUTMANAGER_H
