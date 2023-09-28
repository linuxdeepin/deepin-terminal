// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SETTINGS_H
#define SETTINGS_H

#include <DSettingsDialog>
#include <DKeySequenceEdit>
#include <DComboBox>

#include <QFileSystemWatcher>
#include <qsettingbackend.h>
#include <QSettings>

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

class DSettingsWidgetFactoryPrivate;
/*******************************************************************************
 1. @类名:    Settings
 2. @作者:    ut000439 王培利
 3. @日期:    2020-07-31
 4. @说明:    系统设置
             配置文件：.config/deepin/deepin-terminal/config.conf
*******************************************************************************/
class Settings : public QObject
{
    Q_OBJECT
public:
    static Settings *instance();
    ~Settings();

    /**
     * @brief 设置界面初始化连接
     * @author ut001121 zhangmeng
     */
    void initConnection();

    /**
     * @brief 释放settings实例
     */
    static void releaseInstance();

    /**
     * @brief 设置界面获取透明度的值
     * @author ut001121 zhangmeng
     * @return
     */
    qreal opacity() const;
    /**
     * @brief 设置界面雷神模式的显示/隐藏动画时间
     * @author Archie Meng
     * @return
     */
    int QuakeDuration() const;
    /**
     * @brief 设置界面获取光标形状
     * @author ut001121 zhangmeng
     * @return
     */
    int cursorShape() const;
    /**
     * @brief 设置界面获取光标闪烁属性
     * @author ut001121 zhangmeng
     * @return
     */
    bool cursorBlink() const;
    /**
     * @brief 设置界面获取背景模糊属性
     * @author ut001121 zhangmeng
     * @return
     */
    bool backgroundBlur() const;
    /**
     * @brief 设置界面获取主题颜色
     * @author ut001121 zhangmeng
     * @return
     */
    QString colorScheme() const;
    /**
     * @brief 设置界面获取编码
     * @author ut001121 zhangmeng
     * @return
     */
    QString encoding() const;
    /**
     * @brief 设置界面获取字体名称
     * @author ut001121 zhangmeng
     * @return
     */
    QString fontName();
    /**
     * @brief 设置界面获取字体大小
     * @author ut001121 zhangmeng
     * @return
     */
    int fontSize();
    /**
     * @brief 设置界面获取按键时是否是滚动
     * @author ut001121 zhangmeng
     * @return
     */
    QString wordCharacters() const;
    bool PressingScroll();
    /**
     * @brief 设置界面获取输出时是否是滚动
     * @author ut001121 zhangmeng
     * @return
     */
    bool OutputtingScroll();
    /**
     * @brief 设置界面获取是否允许Ctrl+滚轮扩缩字体
     * @author chenzhiwei
     * @return
     */
    bool ScrollWheelZoom();
//    void reload();
    /**
     * @brief 标签标题
     * @author ut000610 戴正文
     * @return
     */
    QString tabTitleFormat() const;
    /**
     * @brief 远程标签标题
     * @author ut000610 戴正文
     * @return
     */
    QString remoteTabTitleFormat() const;
    /**
     * @brief 获取当前设置选中的shell路径
     * @author ut000610 戴正文
     * @return
     */
    QString shellPath() const;
    /**
     * @brief 重新加载shell配置，并设置当前项
     * @author ut000610 戴正文
     */
    void reloadShellOptions();
    /**
     * @brief 添加默认shell的配置项
     * @author ut000610 戴正文
     */
    static void addShellOption();

    /**
     * @brief 设置界面设置主题颜色
     * @author ut001121 zhangmeng
     * @param name
     */
    void setColorScheme(const QString &name);
    // 设置编码格式, 这个现在限定当前mainwindow生效，不再使用这个全局接口了
//    void setEncoding(const QString &name);
    // 通用设置
//    void setKeyValue(const QString &name, const QString &value);

    /**
     * @brief 获取内置主题
     * @author ut000125 sunchengxi
     * @return
     */
    QString extendColorScheme() const;
    /**
     * @brief 设置内置主题
     * @author ut000125 sunchengxi
     * @param name
     */
    void setExtendColorScheme(const QString &name);
    /**
     * @brief 颜色转字符串
     * @author ut000125 sunchengxi
     * @return
     */
    QStringList color2str(QColor);

    DSettings *settings;

    // 字体下拉列表
    static DComboBox *comboBox;
    // shell配置下拉列表
    static DComboBox *g_shellConfigCombox;

    /**
     * @brief 获取当前配置粘贴是否为选择内容
     * @author n014361 王培利
     * @return
     */
    bool IsPasteSelection();

    /**
     * @brief 与设置里的快捷键冲突检测
     * @author n014361 王培利
     * @param Name 快捷键名称
     * @param Key 快捷键键值
     * @return
     */
    bool isShortcutConflict(const QString &Name, const QString &Key);
    /**
     * @brief 每次显示设置界面时，更新设置的等宽字体
     * @author ut001000 任飞翔
     */
    void handleWidthFont();
    /**
     * @brief 是否禁用Ctrl+S和Ctrl+Q流控制
     * @author 朱科伟
     * @return
     */
    bool disableControlFlow(void);
    /**
     * @brief 历史记录行数
     * @author Archie Meng
     * @return
     */
    int historySize() const;

    //主题名称
    QString themeStr            = "";
    //内置主题名称
    QString extendThemeStr      = "";

    //自定义主题配置文件路径
    QString m_configCustomThemePath;
    //自定义主题配置设置
    QSettings *themeSetting = nullptr;
    //自定义主题配置是否修改
    bool    m_customThemeModify = false;


public:
//    QString getKeyshortcutFromKeymap(const QString &keyCategory, const QString &keyName);
    /**
     * @brief 创建Combox控件
     * @author ut000439 wangpeili
     * @param obj 对象
     * @return
     */
    static QPair<QWidget *, QWidget *> createFontComBoBoxHandle(QObject *obj);
    /**
     * @brief 自定义slider控件样式
     * @author n014361 王培利
     * @param obj 对象
     * @return
     */
    static QPair<QWidget *, QWidget *> createCustomSliderHandle(QObject *obj);
    /**
     * @brief 自定义valSlider控件样式
     * @author Archie Meng
     * @param obj 对象
     * @return
     */
    static QPair<QWidget *, QWidget *> createValSliderHandle(QObject *obj);
    /**
     * @brief 自定义SpinButton控件样式
     * @author ut000439 wangpeili
     * @param obj 对象
     * @return
     */
    static QPair<QWidget *, QWidget *> createSpinButtonHandle(QObject *obj);
    /**
     * @brief 自定义ShortcutEdit控件样式
     * @author ut000439 wangpeili
     * @param opt
     * @return
     */
    static QPair<QWidget *, QWidget *> createShortcutEditOptionHandle(QObject *opt);
    /**
     * @brief 新增自定义修改标签格式的控件
     * @author ut000610 戴正文
     * @param opt
     * @return
     */
    static QPair<QWidget *, QWidget *> createTabTitleFormatOptionHandle(QObject *opt);
    /**
     * @brief 新增自定义修改远程标签格式的控件
     * @author ut000610 戴正文
     * @param opt
     * @return
     */
    static QPair<QWidget *, QWidget *> createRemoteTabTitleFormatOptionHandle(QObject *opt);
    /**
     * @brief 新增自定义shell配置下拉列表控件
     * @author ut000610 戴正文
     * @param opt
     * @return
     */
    static QPair<QWidget *, QWidget *> createShellConfigComboxOptionHandle(QObject *obj);

signals:
    void windowSettingChanged(const QString &key);
    void terminalSettingChanged(const QString &key);
    void shortcutSettingChanged(const QString &key);
    void encodeSettingChanged(const QString &Name);

    void opacityChanged(qreal opacity);
    void cursorShapeChanged(int shape);
    void cursorBlinkChanged(bool blink);
    void backgroundBlurChanged(bool enabled);
    void pressingScrollChanged(bool enabled);
    void OutputScrollChanged(bool enabled);
    void fontSizeChanged(int fontSize);
    void fontChanged(QString fontName);
    void wordCharactersChanged(QString wordCharacters);
    void historySizeChanged(int historySize);

    // 设置中的标签标题格式变化
    void tabFormatChanged(const QString &tabFormat);
    // 设置中的远程标签标题格式变化
    void remoteTabFormatChanged(const QString &remoteTabFormat);

private:
    /**
     * @brief 设置界面初始化
     * @author ut001121 zhangmeng
     */
    void init();

private:
    Settings();
    /**
     * @brief 重新安装终端后在这里重置状态
     * @author ut001121 zhangmeng
     */
    void loadDefaultsWhenReinstall();
    /**
     * @brief 创建标签标题修改界面
     * @author ut000610 戴正文
     * @param opt
     * @param isRemote 是否是远程标签标题
     * @return
     */
    static QPair<QWidget *, QWidget *>  createTabTitleFormatWidget(QObject *opt, bool isRemote);

    static Settings *m_settings_instance;

    Dtk::Core::QSettingBackend *m_backend;
    QString m_configPath;
    //　配置文件监视
    QFileSystemWatcher *m_Watcher = nullptr;
    // 编码格式为当前mainwindow有效参数，不记录在文件中．
    QString m_EncodeName = "UTF-8";
};

/*******************************************************************************
 1. @类名:    KeySequenceEdit
 2. @作者:    ut001121 zhangmeng
 3. @日期:    2020-08-12
 4. @说明:
*******************************************************************************/
class KeySequenceEdit : public DKeySequenceEdit
{
public:
    KeySequenceEdit(DTK_CORE_NAMESPACE::DSettingsOption *opt, QWidget *parent = nullptr): DKeySequenceEdit(parent)
    {
        m_poption = opt;
    }
    DTK_CORE_NAMESPACE::DSettingsOption *option()
    {
        return m_poption;
    }
private:
    DTK_CORE_NAMESPACE::DSettingsOption *m_poption = nullptr;
};

QPair<QWidget *, QWidget *> createShortcutEditOptionHandle(DSettingsWidgetFactoryPrivate *p, QObject *opt);
#define SHORTCUT_VALUE "shortcut_null"
#endif  // SETTINGS_H
