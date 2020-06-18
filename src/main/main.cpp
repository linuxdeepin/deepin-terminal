#include "mainwindow.h"
#include "termargumentparser.h"
#include "termproperties.h"
#include "environments.h"
#include "dbusmanager.h"
#include "service.h"
#include "utils.h"
#include "terminalapplication.h"

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
    //计时
    qint64 starttime = QDateTime::currentMSecsSinceEpoch();
    DApplication::loadDXcbPlugin();

    //DApplication app(argc, argv);
    TerminalApplication app(argc, argv);
    app.loadTranslator();
    app.setOrganizationName("deepin");
    app.setOrganizationDomain("deepin.org");
    app.setApplicationVersion(VERSION);
    app.setApplicationName("deepin-terminal");
    app.setApplicationDisplayName(QObject::tr("Terminal"));
    app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);
    app.setProductIcon(QIcon::fromTheme("deepin-terminal"));
    /***add by ut001121 zhangmeng 20200617 禁用应用程序自动退出 修复BUG33541***/
    app.setQuitOnLastWindowClosed(false);

    QString appDesc = QObject::tr("Terminal is an advanced terminal emulator with workspace"
                                  ", multiple windows, remote management, quake mode and other features.");
    app.setApplicationDescription(appDesc);
    DApplicationSettings set(&app);
#ifdef QT_DEBUG
    QTranslator translator;
    translator.load(QString("deepin-terminal_%1").arg(QLocale::system().name()));
    app.installTranslator(&translator);
#endif  // QT_DEBUG

    /******** Modify by n014361 wangpeili 2020-01-10: 增加日志 ***********×****/
    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();
    /********************* Modify by n014361 wangpeili End *****************/

    // 参数解析
    TermProperties Properties;
    Utils::parseCommandLine(app.arguments(), Properties, true);

    DBusManager manager;
    if (!manager.initDBus()) {
        // 初始化失败，则已经注册过dbus
        // 判断是否能创建新的的窗口
        // 不是雷神且正在创建
        if (!Properties[QuakeMode].toBool() && !Service::instance()->getEnable()) {
            qint64 endtime = QDateTime::currentMSecsSinceEpoch();
            qDebug() << "[sub app] Server can't create, drop this create request! time use "
                     << endtime - starttime <<"ms";
            return 0;
        }

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
        qDebug() << "[sub app] start to call main terminal entry! app args " << args;
        DBusManager::callTerminalEntry(args);
        qint64 endtime2 = QDateTime::currentMSecsSinceEpoch();
        qDebug() << "[sub app] task complete! sub app quit, time use "
                 << endtime2 - starttime<<"ms";
        return 0;
    }
    // 这行不要删除
    qputenv("TERM", "xterm-256color");

    // 主进程
    Service *service = Service::instance();
    service->connect(&manager, &DBusManager::entryArgs, service, &Service::Entry);
    // 初始化数据
    service->init();
    // 创建窗口
    service->Entry(app.arguments());
    qint64 endtime3 = QDateTime::currentMSecsSinceEpoch();
    qDebug() << "First Terminal Window create complete! time use " << endtime3 - starttime <<"ms";

    return app.exec();
}
