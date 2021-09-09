#include "mainwindow.h"
#include <QApplication>
#include "qtermwidget/lib/qtermwidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTermWidget w;
    w.show();

    return a.exec();
}
