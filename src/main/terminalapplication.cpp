#include "terminalapplication.h"
#include "mainwindow.h"
#include "service.h"

// qt
#include <QDebug>

TerminalApplication::TerminalApplication(int &argc, char *argv[]) : DApplication(argc, argv)
{

}

TerminalApplication::~TerminalApplication()
{
    // app结束时，释放Service
    if (nullptr != Service::instance()) {
        delete Service::instance();
    }
}
void TerminalApplication::handleQuitAction()
{
    qDebug() << "handleQuitAction";
    activeWindow()->close();
}
