#ifndef ATSPIDESKTOP_H
#define ATSPIDESKTOP_H
// qt
#include <QThread>

// lib
#include <atspi/atspi.h>
// 该类负责初始化qt-at-spi
// 无障碍辅助工具
// 相关链接 : https://github.com/infapi00/at-spi2-examples/blob/master/c/notify-value-changes.c
// qt-at-spi 快捷键映射依赖此库 让Qt可以识别ctrl + shift + ?快捷键
class AtspiDesktop : public QThread
{
    Q_OBJECT
public:
    AtspiDesktop();
    void stopThread();

    // 事件负责处理g_object
    static void on_event(AtspiEvent *event, void *data);

protected:
    void run() override;

private:
    // 是否进入循环
    bool m_isLoop = false;
};

#endif // ATSPIDESKTOP_H
