#include "dterminalsingleapplication.h"
#include "mainwindow.h"

#include <QLocalSocket>

DTerminalSingleApplication::DTerminalSingleApplication(int &argc, char *argv[], const QString uniqueKey) : DApplication(argc, argv), _uniqueKey(uniqueKey)
{
    sharedMemory.setKey(_uniqueKey);
    if (sharedMemory.attach())
    {
        _isRunning = true;
    }
    else
    {
        _isRunning = false;
        // create shared memory.
        if (!sharedMemory.create(1))
        {
            qDebug("Unable to create single instance.");
            return;
        }

        // create local server and listen to incomming messages from other instances.
        localServer = new QLocalServer(this);
        connect(localServer, SIGNAL(newConnection()), this, SLOT(receiveMessage()));
        localServer->listen(_uniqueKey);
    }
}

//fix bug 22670
bool DTerminalSingleApplication::isRunning()
{
    DTerminalSingleApplication *dApp = (static_cast<DTerminalSingleApplication*>(QCoreApplication::instance()));
    QString cachePath = QStandardPaths::standardLocations(QStandardPaths::CacheLocation).at(0);
    QFile processFile(QString("%1/%2").arg(cachePath).arg("process.pid"));

    if (processFile.exists()) {
        if (processFile.open(QIODevice::ReadWrite)) {
            int historyId = processFile.readAll().toInt();
            QDir hisProcessDir(QString("/proc/%1").arg(historyId));
            processFile.close();
            // use /proc/pid to judge process running state
            if (hisProcessDir.exists()) {
                return true;
            }

            if (processFile.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
                QTextStream pidInfo(&processFile);
                pidInfo << dApp->applicationPid();
                processFile.close();
            } else {
                qDebug() << "process File open failed!";
            }
        }
    } else {
        if (processFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream pidInfo(&processFile);
            pidInfo << dApp->applicationPid();
            processFile.close();
        } else {
            qDebug() << "process File open failed!";
        }
    }

    return false;
}

// public slots.
void DTerminalSingleApplication::receiveMessage()
{
    QLocalSocket *localSocket = localServer->nextPendingConnection();
    if (!localSocket->waitForReadyRead(timeout))
    {
        qDebug(localSocket->errorString().toLatin1());
        return;
    }

    QByteArray byteArray = localSocket->readAll();
    QString message = QString::fromUtf8(byteArray.constData());
    emit messageAvailable(message);
    localSocket->disconnectFromServer();
}

bool DTerminalSingleApplication::sendMessage(const QString &message)
{
    if (!_isRunning)
    {
        return false;
    }
    QLocalSocket localSocket(this);
    localSocket.connectToServer(_uniqueKey, QIODevice::WriteOnly);
    if (!localSocket.waitForConnected(timeout))
    {
        qDebug(localSocket.errorString().toLatin1());
        return false;
    }

    localSocket.write(message.toUtf8());
    if (!localSocket.waitForBytesWritten(timeout))
    {
        qDebug(localSocket.errorString().toLatin1());
        return false;
    }

    localSocket.disconnectFromServer();
    return true;
}

void DTerminalSingleApplication::handleQuitAction()
{
    foreach (MainWindow *window, MainWindow::getWindowList())
    {
        window->close();
    }
}
