#include "widget.h"

#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.show();

    vte::base::Ring *ring = new vte::base::Ring();

    ring->reset();

    qDebug() << ring->length();

    return a.exec();
}
