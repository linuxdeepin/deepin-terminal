#include "mainwindow.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
    DApplication::loadDXcbPlugin();

    DApplication app(argc, argv);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    app.loadTranslator();

    qputenv("TERM", "xterm-256color");

    MainWindow w;
    w.show();

    return app.exec();
}
