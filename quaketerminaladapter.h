#ifndef QUAKETERMINALADAPTER_H
#define QUAKETERMINALADAPTER_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>

#define kQuakeTerminalService "com.deepin.quake_terminal"
#define kQuakeTerminalIface "/com/deepin/quake_terminal"

/*
 * Adaptor class for interface com.deepin.quake_terminal
 */
class QuakeTerminalAdapter : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.quake_terminal")
    Q_CLASSINFO("D-Bus Introspection", ""
                                       "  <interface name=\"com.deepin.quake_terminal\">\n"
                                       "    <method name=\"ShowOrHide\">\n"
                                       "    </method>\n"
                                       "  </interface>\n"
                                       "")
public:
    QuakeTerminalAdapter(QObject *parent);
    virtual ~QuakeTerminalAdapter();

public Q_SLOTS:
    void ShowOrHide();
};

#endif  // QUAKETERMINALADAPTER_H
