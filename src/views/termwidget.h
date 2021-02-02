/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:     wangpeili <wangpeili@uniontech.com>
 *
 * Maintainer: wangpeili <wangpeili@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef TERMWIDGET_H
#define TERMWIDGET_H

#include "qtermwidget.h"
#include "termwidgetpage.h"

/*******************************************************************************
 1. @类名:    TermWidget
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:
*******************************************************************************/

// 程序名 %n
const QString PROGRAM_NAME = "%n";
// 当前目录长 %D
const QString DIR_L = "%D";
// 当前目录短 %d
const QString DIR_S = "%d";
// 会话编号 %#
const QString TAB_NUM = "%#";
// 用户名
const QString USER_NAME = "%u";
// 用户名@
const QString USER_NAME_L = "%U";
// 本地主机
const QString LOCAL_HOST_NAME = "%h";
// 远程主机
const QString REMOTE_HOST_NAME = "%h";
// shell设置的窗口标题
const QString SHELL_TITLE = "%w";

// 标签标题格式参数
const QString TAB_ARGS = "%n %d %D %# %u %h %w";
// 远程标签标题格式参数
const QString REMOTE_ARGS = "%u %U %h %# %w";

// 删除和退格键可选模式
enum EraseMode {
    EraseMode_Auto,
    EraseMode_Ascii_Delete,
    EraseMode_Control_H,
    EraseMode_TTY,
    EraseMode_Escape_Sequeue
};

// 标签标题格式
struct TabFormat {
    // 当前标签标题格式
    QString currentTabFormat;
    // 远程标签标题格式
    QString remoteTabFormat;
    // 是否是全局设置
    bool isGlobal = true;
};

class TermProperties;
struct ServerConfig;
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

    bool enterSzCommand() const;
    void setEnterSzCommand(bool enterSzCommand);

    bool isConnectRemote() const;
    void setIsConnectRemote(bool isConnectRemote);
    // 连接远程后修改标签标题
    void modifyRemoteTabTitle(ServerConfig remoteConfig);

    QString encode() const;
    void setEncode(const QString &encode);

    QString RemoteEncode() const;
    void setRemoteEncode(const QString &RemoteEncode);

    // 设置退格键模式（用户选择接口）
    void setBackspaceMode(const EraseMode &backspaceMode);

    // 设置删除键模式（用户选择接口）
    void setDeleteMode(const EraseMode &deleteMode);

    // 获取该终端距离page的层次
    int getTermLayer();

    // 修改当前标签格式
    void setTabFormat(const QString &tabFormat);
    // 修改远程标签格式
    void setRemoteTabFormat(const QString &remoteTabFormat);
    // 修改所有的标签标题格式
    void renameTabFormat(const QString &tabFormat, const QString &remoteTabFormat);
    // 获取标签标题对应的标签标题
    QString getTabTitle();
    // 初始化终端标签
    void initTabTitle();
    // 初始化标签标题参数列表
    void initTabTitleArgs();
    // 获取标签标题格式
    QString getTabTitleFormat();
    // 获取远程标签标题格式
    QString getRemoteTabTitleFormat();
    // 获取当前标签标题格式
    QString getCurrentTabTitleFormat();
    // Ctrl+Q/Ctrl+S的悬浮提示框
    void showFlowMessage(bool show);
    // shell启动失败悬浮框
    void showShellMessage(QString strWarnings);

public slots:
    void wpasteSelection();
    void onSettingValueChanged(const QString &keyName);
    // 处理拖拽进来的文件名Urls
    void onDropInUrls(const char *urls);
    // 处理触控板事件
    void onTouchPadSignal(QString name, QString direction, int fingers);
    // 处理shell消息提示
    void onShellMessage(QString currentShell, bool isSuccess);

signals:
    void termRequestRenameTab(QString newTabName);
    void termIsIdle(QString tabIdentifier, bool bIdle);
    void termTitleChanged(QString titleText);

protected:
    // 鼠标滚轮事件
    void wheelEvent(QWheelEvent *event) override;
private slots:
    void customContextMenuCall(const QPoint &pos);
    void handleTermIdle(bool bIdle);
    // 处理标签标题参数变化
    void onTitleArgsChange(QString key, QString value);
    // hostname名称变化
    void onHostnameChanged();

    void onQTermWidgetReceivedData(QString value);
    void onTermWidgetReceivedData(QString value);
    void onExitRemoteServer();
    void onUrlActivated(const QUrl & url, bool fromContextMenu);
    void onThemeTypeChanged(DGuiApplicationHelper::ColorType builtInTheme);
    void onTermIsIdle(bool bIdle);
    void onTitleChanged();
    void onWindowEffectEnabled(bool isWinEffectEnabled);
    void onCopyAvailable(bool enable);
    void onSetTerminalFont();
    void onSig_matchFound();
    void onSig_noMatchFound();

    void onCopy();
    void onPaste();
    void onOpenFile();
    void onOpenFileInFileManager();
    void onUploadFile();
    void onDownloadFile();
    void onShowSettings();
    void onShowEncoding();
    void onShowCustomCommands();
    void onShowRemoteManagement();
    void onShowSearchBar();
    void onHorizontalSplit();
    void onVerticalSplit();
    void splitHorizontal();
    void splitVertical();
    void onCloseCurrWorkSpace();
    void onCloseOtherWorkSpaces();
    void onCreateNewTab();
    void onSwitchFullScreen();
    void openBing();
    void openBaidu();
    void openGithub();
    void openStackOverflow();
private:
    /*** 修复 bug 28162 鼠标左右键一起按终端会退出 ***/
    void addMenuActions(const QPoint &pos);
    // 根据标签格式获取标签标题
    QString getTabTitle(QMap<QString, QString> format, QString TabFormat);
    //分屏时切换到当前选中主题方案
    void switchThemeOnSplitScreen();
    void openUrl(QString strUrl);
    //根据选择的文件名字符串得到合法的文件名，去除文件名开头/结尾的''或""
    QString getFormatFileName(QString selectedText);
    //根据文件名拼接得到文件路径
    QString getFilePath(QString fileName);

    TermWidgetPage *m_page = nullptr;
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
    QString m_remoteEncode = "UTF-8";
    // 当前终端退格信号
    EraseMode m_backspaceMode = EraseMode_Ascii_Delete;
    // 当前终端删除信号
    EraseMode m_deleteMode = EraseMode_Escape_Sequeue;

    // 标签格式
    // 当前标签标题参数
    QMap<QString, QString> m_tabArgs;
    // 远程标签标题参数
    QMap<QString, QString> m_remoteTabArgs;
    // 标签标题格式
    TabFormat m_tabFormat;
    // 会话编号
    int m_sessionNumber;
    // Ctrl+Q/Ctrl+S的悬浮框
    DFloatingMessage *m_flowMessage = nullptr;
};

#endif  // TERMWIDGET_H
