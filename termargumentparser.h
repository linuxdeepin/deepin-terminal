#ifndef TERMARGUMENTPARSER_H
#define TERMARGUMENTPARSER_H

#include "mainwindow.h"
#include "termproperties.h"

#include <QObject>

class QuakeTerminalProxy;

class TermArgumentParser : public QObject
{
    Q_OBJECT
public:
    explicit TermArgumentParser(QObject *parent = nullptr);
    ~TermArgumentParser() override;

    bool initDBus();
    bool parseArguments(MainWindow *mainWindow);

    void showOrHideQuakeTerminal();

private:
    MainWindow *m_mainWindow{ nullptr };

    QuakeTerminalProxy *m_quakeTerminalProxy{ nullptr };
};

#endif  // TERMARGUMENTPARSER_H
