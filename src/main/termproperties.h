#ifndef TERMPROPERTIES_H
#define TERMPROPERTIES_H

#include <QVariant>

enum TermProperty {
    SingleFlag,        // mainwindow使用, 默认为false
    QuakeMode,         // mainwindow使用, 右键菜单要用到．
    WorkingDir,        // 每个terminal单独使用
    ColorScheme,       // 未使用
    ShellProgram,      // 仅供第一个terminal使用
    Execute,           // 仅供第一个terminal使用，任意长，任意位置，QStringList
    StartWindowState,  // mainwindow使用
    KeepOpen,          // 仅供第一个terminal使用
    Script             // 仅供第一个terminal使用
};

class TermProperties
{
    Q_GADGET
public:
    explicit TermProperties() = default;
    explicit TermProperties(QString workingDir);
    explicit TermProperties(QString workingDir, QString colorScheme);
    explicit TermProperties(QMap<TermProperty, QVariant> list);

    bool contains(TermProperty propertyType) const;

    void setTermPropertyMap(QMap<TermProperty, QVariant> list);
    void setWorkingDir(QString workingDir);

    QVariant &operator[](const TermProperty &key);
    const QVariant operator[](const TermProperty &key) const;

private:
    QMap<TermProperty, QVariant> m_properties;
};

#endif  // TERMPROPERTIES_H
