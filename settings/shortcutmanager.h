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
    QString getClipboardCommandData();

    // 判断快捷键是否合法可用，进行界面处理
    bool isValidShortcut(const QString &Name, const QString &Key);
    // 检测快捷键是否合法可用，无界面
    bool checkShortcutValid(const QString &Name, const QString &Key, QString &Reason);
    // 快捷键是否已被自定义设置
    bool isShortcutConflictInCustom(const QString &Name, const QString &Key);

private:
    MainWindow *m_mainWindow = nullptr;
    QList<QAction *> m_customCommandActionList;
    QStringList m_builtinShortcuts;
    static ShortcutManager *m_instance;
    QString m_clipboardCommand;
};

#endif  // SHORTCUTMANAGER_H
