#include "mainwindow.h"
#include "termargumentparser.h"
#include "termproperties.h"
#include "environments.h"
#include "dbusmanager.h"
#include "service.h"
#include "utils.h"

#include <DApplication>
#include <DApplicationSettings>
#include <DLog>

#include <QDir>
#include <QDebug>
#include <QCommandLineParser>
#include <QTranslator>

DWIDGET_USE_NAMESPACE
/******** Modify by n014361 wangpeili 2020-01-10:增加日志需要 ***********×****/
DCORE_USE_NAMESPACE
/********************* Modify by n014361 wangpeili End ************************/
int main(int argc, char *argv[])
{
    DApplication::loadDXcbPlugin();

    //DApplication app(argc, argv);
    DApplication app(argc, argv);
    app.setOrganizationName("deepin");
    app.setOrganizationDomain("deepin.org");
    app.setApplicationVersion(VERSION);
    app.setApplicationName("deepin-terminal");
    app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);
    app.setProductIcon(QIcon::fromTheme("deepin-terminal"));
    app.loadTranslator();

    QString appDesc = QObject::tr("Terminal is an advanced terminal emulator with workspace"
                                  ", multiple windows, remote management, quake mode and other features.");
    app.setApplicationDescription(appDesc);
    DApplicationSettings set(&app);
#ifdef QT_DEBUG
    QTranslator translator;
    translator.load(QString("deepin-terminal_%1").arg(QLocale::system().name()));
    app.installTranslator(&translator);
#endif  // QT_DEBUG

    app.setApplicationDisplayName(QObject::tr("Terminal"));

    qputenv("TERM", "xterm-256color");

    /******** Modify by n014361 wangpeili 2020-01-10: 增加日志 ***********×****/
    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();
    /********************* Modify by n014361 wangpeili End *****************/

    QCommandLineParser parser;
    Utils::setCommandLineParser(appDesc, app, parser);
    parser.process(app);

    DBusManager manager;
    if (!manager.initDBus()) {
        // 初始化失败，则已经注册过dbus
        // 调用entry接口
        /******** Modify by ut000610 daizhengwen 2020-05-25: 在终端中打开****************/
        QStringList args = app.arguments();
        bool isCurrentPaht = false;
        for (QString arg : args) {
            // 若已有-w和--work-directory参数，直接将参数传给主进程执行
            if (arg == "-w" || arg == "--work-directory") {
                isCurrentPaht = true;
                break;
            }
        }
        if (!isCurrentPaht) {
            args += "-w";
            args += QDir::currentPath();
        }
        /********************* Modify by ut000610 daizhengwen End ************************/
        qDebug() << "app args " << args;
        DBusManager::callTerminalEntry(args);
        return 0;
    }

    // 主进程
    Service *service = Service::instance();
    service->connect(&manager, &DBusManager::entryArgs, service, &Service::Entry);
    // 初始化数据
    service->init();
    // 创建窗口
    service->Entry(app.arguments());

    return app.exec();
}
