#include "mainwindow.h"

#include "termproperties.h"

#include <DApplication>

#include <QDebug>
#include <QCommandLineParser>
#include <QTranslator>

DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
    DApplication::loadDXcbPlugin();

    DApplication app(argc, argv);
    app.setOrganizationName("blumia");
    app.setApplicationName("dterm");
    app.setApplicationDisplayName(QObject::tr("Deep Dark Terminal"));
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    app.setProductIcon(QIcon::fromTheme("deepin-terminal"));
    app.loadTranslator();

#ifdef QT_DEBUG
    QTranslator translator;
    translator.load(QString("dterm_%1").arg(QLocale::system().name()));
    app.installTranslator(&translator);
#endif // QT_DEBUG

    qputenv("TERM", "xterm-256color");

    QCommandLineParser parser;
    parser.addHelpOption();
    QCommandLineOption optionExecute({"e", "execute"}, "Execute command instead of shell", "command");
    parser.addOptions({optionExecute});
    parser.process(app);

    TermProperties firstTermProperties;

    if (parser.isSet(optionExecute)) {
        // get raw arg, so we cannot use `parser.value(optionExecute)`.
        QStringList rawCommandList;
        bool reachedArg = false;
        for (int i = 0; i < argc; i++) {
            QString currentArg = QString::fromLocal8Bit(argv[i]);
            if (reachedArg) {
                rawCommandList.append(currentArg);
            } else {
                if (currentArg == "-e" || currentArg == "--execute") {
                    reachedArg = true;
                    continue;
                }
            }
        }
        firstTermProperties[Execute] = rawCommandList.join(' ');
    }

    MainWindow w(firstTermProperties);
    w.show();

    return app.exec();
}
