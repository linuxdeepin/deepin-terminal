#include "terminalapplication.h"
#include "mainwindow.h"
#include <QDebug>

TerminalApplication::TerminalApplication(int &argc, char *argv[]) : DApplication(argc, argv)
{

}
void TerminalApplication::handleQuitAction()
{
    qDebug() << "handleQuitAction";
    activeWindow()->close();
}
