#ifndef QUAKETERMINALPROXY_H
#define QUAKETERMINALPROXY_H

#include <DSysInfo>

#include <QDBusConnection>
#include <QObject>

class QuakeTerminalProxy : public QObject
{
    Q_OBJECT
public:
    explicit QuakeTerminalProxy(QObject *parent = nullptr);
    ~QuakeTerminalProxy();

public slots:
    void ShowOrHide();
    void hidePlugin();
};

#endif  // QUAKETERMINALPROXY_H
