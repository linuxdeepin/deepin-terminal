#ifndef DBUSLOGIN_H
#define DBUSLOGIN_H
#include "termproperties.h"
#include "utils.h"

#include <QObject>
#include <QDBusConnection>
#include <QVariant>
#include <QList>

#define MAXWINDOWCOUNT 199

class TermProperties;
class MainWindow;
class DBusLogin : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", TERMINALSERVER)
public:
    DBusLogin();
    bool initDBus();
    void doCreateNewWindow(TermProperties properties);

public slots:
    void onCreateNewWindow(QList<QVariant> args);
    void onCloseWindow(int index);
    void onShoworHidePlugin(int index, const QString &pluginName);

private slots:
    void onShowWindow(int index);

private:
    bool isValid(TermProperty index, TermProperties &properties);
    void getProperty(TermProperty index, TermProperties &properties, QString args);

    QList<MainWindow *> m_mainWindowList;
    bool m_createMutex = true;
};

#endif // DBUSLOGIN_H
