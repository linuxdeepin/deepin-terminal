#include "quaketerminaladapter.h"

#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QMetaObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class QuakeTerminalAdapter
 */

QuakeTerminalAdapter::QuakeTerminalAdapter(QObject *parent) : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

QuakeTerminalAdapter::~QuakeTerminalAdapter()
{
    // destructor
}

void QuakeTerminalAdapter::ShowOrHide()
{
    qDebug() << "QuakeTerminalAdapter::ShowOrHide";
    QMetaObject::invokeMethod(parent(), "ShowOrHide");
}
