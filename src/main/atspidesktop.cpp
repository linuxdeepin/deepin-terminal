#include "atspidesktop.h"

//qt
#include <QDebug>

AtspiDesktop::AtspiDesktop()
{

}

/*******************************************************************************
 1. @函数:    stopThread
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-09
 4. @说明:    负责退出spi事件循环
*******************************************************************************/
void  AtspiDesktop::stopThread()
{
    // 初始化为成功直接退出
    if (!m_isLoop) {
        qDebug() << "AtspiDesktop thread atspi_event_main do not run";
        return;
    }
    // 退出插件循环
    atspi_event_quit();
    // 加上会报错
    // atspi_exit();
    // 等待退出线程
    quit();
    wait();
    qDebug() << "quit AtspiDesktop thread!";
    m_isLoop = false;
}

/*******************************************************************************
 1. @函数:    on_event
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-09
 4. @说明:    注册监听事件需要此函数
             需要对g_object进行处理
*******************************************************************************/
void AtspiDesktop::on_event(AtspiEvent *event, void *data)
{
    g_object_unref(event->source);
    g_free(event->type);
    g_value_unset(&event->any_data);
    g_free(event);
    Q_UNUSED(data);
}

void AtspiDesktop::run()
{
    // 设置accessibility参数
    char cmdGsettings[128] = "gsettings set org.gnome.desktop.interface toolkit-accessibility true";
    if (0 == system(cmdGsettings)) {
        qDebug() << "exec:[gsettings set org.gnome.desktop.interface toolkit-accessibility true]success";
    } else {
        qDebug() << "exec gsettings failed";
    }

    int result = atspi_init();
    qDebug()  << "atspi_init result : " << result;
    if (result != 0) {
        // 初始化失败
        qDebug() << "atspi_init failed : " << result;
        return;
    }
    AtspiEventListener *listener = atspi_event_listener_new(on_event, NULL, NULL);
    if (listener) {
        qDebug() << "object:state-changed:focused" << atspi_event_listener_register(listener, "object:state-changed:focused", NULL);
        m_isLoop = true;
        // 主循环
        atspi_event_main();

    }
}
