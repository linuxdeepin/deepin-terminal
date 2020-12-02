/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:     zhangmeng <zhangmeng@uniontech.com>
 *
 * Maintainer: zhangmeng <zhangmeng@uniontech.com>
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
#ifndef SETTINGS_H
#define SETTINGS_H

#include <DSettingsDialog>
#include <DKeySequenceEdit>

#include <QFileSystemWatcher>
#include <qsettingbackend.h>
#include <QSettings>

#include <DComboBox>

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
    void init();

    void initConnection();

    qreal opacity() const;
    int cursorShape() const;
    bool cursorBlink() const;
    bool backgroundBlur() const;
    QString colorScheme() const;
    QString encoding() const;
    QString fontName();
    int fontSize();
    bool PressingScroll();
    bool OutputtingScroll();
    void reload();
    // 标签标题
    QString tabTitleFormat() const;
    // 远程标签标题
    QString remoteTabTitleFormat() const;
    // 当前选中的shell路径
    QString shellPath() const;
    // 重新加载shell配置
    void reloadShellOptions();
    // 为默认shell添加选项
    static void addShellOption();

    // 设置主题
    void setColorScheme(const QString &name);
    // 设置编码格式, 这个现在限定当前mainwindow生效，不再使用这个全局接口了
    void setEncoding(const QString &name);
    // 通用设置
    void setKeyValue(const QString &name, const QString &value);

    //获取内置主题
    QString extendColorScheme() const;
    //设置内置主题
    void setExtendColorScheme(const QString &name);
    //颜色转字符串
    QStringList color2str(QColor);

    DSettings *settings;

    // 字体下拉列表
    static DComboBox *comboBox;
    // shell配置下拉列表
    static DComboBox *g_shellConfigCombox;

    bool IsPasteSelection();

    // 与设置里的快捷键冲突检测
    bool isShortcutConflict(const QString &Name, const QString &Key);
    void HandleWidthFont();
    //是否启动Ctrl+S/Ctrl+Q流控制
    bool enableControlFlow(void);

    //是否选择了主题
    bool bSwitchTheme           = false;
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
    QString getKeyshortcutFromKeymap(const QString &keyCategory, const QString &keyName);

    static QPair<QWidget *, QWidget *> createFontComBoBoxHandle(QObject *obj);
    // 新增自定义slider控件
    static QPair<QWidget *, QWidget *> createCustomSliderHandle(QObject *obj);
    // 新增自定义spinbutton控件
    static QPair<QWidget *, QWidget *> createSpinButtonHandle(QObject *obj);
    // 新增自定义ShortcutEdit控件处理
    static QPair<QWidget *, QWidget *> createShortcutEditOptionHandle(QObject *opt);
    // 新增自定义修改标签格式的控件
    static QPair<QWidget *, QWidget *> createTabTitleFormatOptionHandle(QObject *opt);
    // 新增自定义修改远程标签格式的控件
    static QPair<QWidget *, QWidget *> createRemoteTabTitleFormatOptionHandle(QObject *opt);
    // 新增自定义shell配置下拉列表控件
    static QPair<QWidget *, QWidget *> createShellConfigComboxOptionHandle(QObject *opt);

signals:
    // void settingValueChanged(const QString &key, const QVariant &value);
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

    // 设置中的标签标题格式变化
    void tabFormatChanged(const QString &tabFormat);
    // 设置中的远程标签标题格式变化
    void remoteTabFormatChanged(const QString &remoteTabFormat);

private:
    Settings();
    void loadDefaultsWhenReinstall();
    // 创建标题格式输入控件
    static QPair<QWidget *, QWidget *>  createTabTitleFormatWidget(QObject *opt, bool isRemote);

    static Settings *m_settings_instance;

    Dtk::Core::QSettingBackend *m_backend;
    QString m_configPath;
    //　配置文件监视
    QFileSystemWatcher * m_Watcher = nullptr;
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
