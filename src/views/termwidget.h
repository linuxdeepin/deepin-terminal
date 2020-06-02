#ifndef TERMWIDGET_H
#define TERMWIDGET_H

#include "qtermwidget.h"
#include "termwidgetpage.h"

// 删除和退格键可选模式
enum EraseMode {
    EraseMode_Auto,
    EraseMode_Ascii_Delete,
    EraseMode_Control_H,
    EraseMode_TTY,
    EraseMode_Escape_Sequeue
};

class TermProperties;
class TermWidget : public QTermWidget
{
    Q_OBJECT
public:
    TermWidget(TermProperties properties, QWidget *parent = nullptr);
    ~TermWidget();
    TermWidgetPage *parentPage();
    bool isInRemoteServer();
public:
    // 跳转到下一个命令
    void skipToNextCommand();
    // 跳转到前一个命令
    void skipToPreCommand();
    void setTermOpacity(qreal opacity);
    // 修改字体
    void setTermFont(const QString &fontName);
    // 修改字体大小
    void setTermFontSize(const int fontSize);
    // 修改光标形状
    void setCursorShape(int shape);
    void setPressingScroll(bool enable);
    // 设置编码
    void selectEncode(QString encode);
    // session支持最大值．再多了就起不来了．
    static const int MaxTermwidgetCount = 199;

    bool enterSzCommand() const;
    void setEnterSzCommand(bool enterSzCommand);

    bool isConnectRemote() const;
    void setIsConnectRemote(bool isConnectRemote);

    QString encode() const;
    void setEncode(const QString &encode);

    QString RemoteEncode() const;
    void setRemoteEncode(const QString &RemoteEncode);

    // 设置退格键模式（用户选择接口）
    void setBackspaceMode(const EraseMode &backspaceMode);

    // 设置删除键模式（用户选择接口）
    void setDeleteMode(const EraseMode &deleteMode);

public slots:
    void wpasteSelection();
    void onSettingValueChanged(const QString &keyName);

signals:
    void termRequestRenameTab(QString newTabName);
    void termIsIdle(int currSessionId, bool bIdle);
    void termTitleChanged(QString titleText);

private slots:
    void customContextMenuCall(const QPoint &pos);
    void handleTermIdle(bool bIdle);
private:
    /*** 修复 bug 28162 鼠标左右键一起按终端会退出 ***/
    void addMenuActions(const QPoint &pos);

    TermWidgetPage *m_Page = nullptr;
    TermProperties m_properties;

    DMenu *m_menu = nullptr;
    /******** Modify by ut000610 daizhengwen 2020-05-27: 当前窗口是否要进行下载操作****************/
    bool m_enterSzCommand = false;
    /********************* Modify by ut000610 daizhengwen End ************************/
    /******** Modify by ut000610 daizhengwen 2020-05-28: 当前窗口是否连接远程服务器****************/
    bool m_isConnectRemote = false;
    /********************* Modify by ut000610 daizhengwen End ************************/
    // 当前编码
    QString m_encode = "UTF-8";
    // 远程编码
    QString m_RemoteEncode = "UTF-8";
    // 当前终端退格信号
    EraseMode m_backspaceMode = EraseMode_Ascii_Delete;
    // 当前终端删除信号
    EraseMode m_deleteMode = EraseMode_Escape_Sequeue;
};

#endif  // TERMWIDGET_H
