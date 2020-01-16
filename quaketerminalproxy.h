#ifndef QUAKETERMINALPROXY_H
#define QUAKETERMINALPROXY_H

#include <QDBusConnection>
#include <QObject>

#include <DSysInfo>

class QuakeTerminalProxy : public QObject
{
    Q_OBJECT
public:
    explicit QuakeTerminalProxy(QObject *parent = nullptr);
    ~QuakeTerminalProxy();

public slots:
    void ShowOrHide();
};

#endif  // QUAKETERMINALPROXY_H
