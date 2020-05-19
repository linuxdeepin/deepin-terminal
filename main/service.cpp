#include "service.h"
#include "windowsmanager.h"
#include "utils.h"

#include <QDebug>

Service *Service::pService = new Service();
Service *Service::instance()
{
    return  pService;
}

void Service::init()
{

}

void Service::Entry(QStringList arguments)
{
    TermProperties properties = Utils::parseArgument( arguments);
    // 雷神处理入口
    if (properties[QuakeMode].toBool()) {
        WindowsManager::instance()->runQuakeWindow(properties);
        return;
    }
    // 普通窗口处理入口
    WindowsManager::instance()->createNormalWindow(properties);
    return;
}

Service::Service(QObject *parent) : QObject(parent)
{

}
