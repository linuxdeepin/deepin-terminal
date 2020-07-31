#ifndef SETTINGS_H
#define SETTINGS_H

#include <DSettingsDialog>
#include <DKeySequenceEdit>

#include <QFileSystemWatcher>
#include <qsettingbackend.h>

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

    // 设置主题
    void setColorScheme(const QString &name);
    // 设置编码格式, 这个现在限定当前mainwindow生效，不再使用这个全局接口了
    void setEncoding(const QString &name);
    // 通用设置
    void setKeyValue(const QString &name, const QString &value);

    DSettings *settings;
    static DComboBox *comboBox;
    bool IsPasteSelection();

    // 与设置里的快捷键冲突检测
    bool isShortcutConflict(const QString &Name, const QString &Key);
    void HandleWidthFont();

public:
    QString getKeyshortcutFromKeymap(const QString &keyCategory, const QString &keyName);

    static QPair<QWidget *, QWidget *> createFontComBoBoxHandle(QObject *obj);
    // 新增自定义slider控件
    static QPair<QWidget *, QWidget *> createCustomSliderHandle(QObject *obj);
    // 新增自定义spinbutton控件
    static QPair<QWidget *, QWidget *> createSpinButtonHandle(QObject *obj);
    // 新增自定义ShortcutEdit控件处理
    static QPair<QWidget *, QWidget *> createShortcutEditOptionHandle(QObject *opt);

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

private:
    Settings();
    void loadDefaultsWhenReinstall();

    static Settings *m_settings_instance;

    Dtk::Core::QSettingBackend *m_backend;
    QString m_configPath;
    //　配置文件监视
    QFileSystemWatcher * m_Watcher = nullptr;
    // 编码格式为当前mainwindow有效参数，不记录在文件中．
    QString m_EncodeName = "UTF-8";
};
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
