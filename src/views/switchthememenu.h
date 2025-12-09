#ifndef SWITCHTHEMEMENU_H
#define SWITCHTHEMEMENU_H

#include <QMenu>
#include <QEvent>
#include <QKeyEvent>

/*******************************************************************************
 1. @类名:    SwitchThemeMenu
 2. @作者:    ut000125 sunchengxi
 3. @日期:    2020-10-28
 4. @说明:    主题菜单的快捷键项在鼠标离开悬浮时，触发主题还原
*******************************************************************************/
class SwitchThemeMenu : public QMenu
{
    Q_OBJECT
public:
    SwitchThemeMenu(const QString &title, QWidget *parent = nullptr);
    /**
     * @brief 捕获鼠标离开主题项事件
     * @author ut000125 sunchengxi
     */
    void leaveEvent(QEvent *) override;
    /**
     * @brief 主题菜单栏隐藏时触发
     * @author ut000125 sunchengxi
     */
    void hideEvent(QHideEvent *) override;
    /**
     * @brief 捕获鼠标进入主题项事件
     * @author ut000125 sunchengxi
     * @param event 鼠标进入主题项事件
     */
    void enterEvent(QEvent *event) override;
    /**
     * @brief 处理键盘主题项左键按下离开事件
     * @author ut000125 sunchengxi
     * @param event 键盘主题项左键按下离开事件
     */
    void keyPressEvent(QKeyEvent *event) override;

signals:
    //主题项在鼠标停靠离开时触发的信号
    void mainWindowCheckThemeItemSignal();
    //主题菜单隐藏时设置主题信号
    void menuHideSetThemeSignal();
public:
    //鼠标悬殊主题记录，防止频繁刷新，鼠标再次进入主题列表负责刷新预览
    QString hoveredThemeStr = "";
};
#endif // SWITCHTHEMEMENU_H
