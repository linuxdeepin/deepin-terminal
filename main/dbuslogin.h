#ifndef DBUSLOGIN_H
#define DBUSLOGIN_H
#include "termproperties.h"

#include <QObject>
#include <QDBusConnection>
#include <QVariant>
#include <QList>

#define MAXWINDOWCOUNT 30
#define DEEPINDEBUSNAME "com.deepin.terminal.reborn"
class TermProperties;
class MainWindow;

class DBusLogin : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", DEEPINDEBUSNAME)
public:
    DBusLogin();
    bool initDBus();
    void doCreateNewWindow(TermProperties properties);

public slots:
    void onCreateNewWindow(QList<QVariant> args);

private slots:
    void onShowWindow(int index);

private:
    bool isValid(TermProperty index, TermProperties &properties);
    void getProperty(TermProperty index, TermProperties &properties, QString args);

    QList<MainWindow *> m_mainWindowList;
    bool m_createMutex = true;
};

#endif // DBUSLOGIN_H
