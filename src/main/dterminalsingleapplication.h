#ifndef DTERMINALSINGLEAPPLICATION_H
#define DTERMINALSINGLEAPPLICATION_H

#include <DApplication>

#include <QSharedMemory>
#include <QLocalServer>

DWIDGET_USE_NAMESPACE

class DTerminalSingleApplication : public DApplication

{
    Q_OBJECT
public:
    DTerminalSingleApplication(int &argc, char *argv[], const QString uniqueKey);
    bool isRunning();
    bool sendMessage(const QString &message);

protected:
    void handleQuitAction() override;

public slots:
    void receiveMessage();
signals:
    void messageAvailable(QString message);
private:
    bool _isRunning;
    QString _uniqueKey;
    QSharedMemory sharedMemory;
    QLocalServer *localServer;
    static const int timeout = 1000;
};

#endif // DTERMINALSINGLEAPPLICATION_H
