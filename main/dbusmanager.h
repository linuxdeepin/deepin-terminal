#ifndef DBUSMANAGER_H
#define DBUSMANAGER_H
#include "termproperties.h"

#include <QObject>
#include <QList>
#include <QVariant>

// deepin-terminal
#define TERMINALSERVER "com.deepin.terminal.reborn"
#define TERMINALINTERFACE "/window"

// kwin dbus
#define KWinDBusService "org.kde.KWin"
#define KWinDBusPath "/KWin"

class DBusManager : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", TERMINALSERVER)
public:
    DBusManager();
    // 初始化terminal相关dbus
    bool initDBus();

    // kwin dbus
    static int callKDECurrentDesktop();
    static void callKDESetCurrentDesktop(int index);
    // deepin terminal
    static void callTerminalEntry(QStringList args);

signals:
    // To Service entry function
    void entryArgs(QStringList args);

public slots:
    void entry(QStringList args);
};

#endif // DBUSMANAGER_H
