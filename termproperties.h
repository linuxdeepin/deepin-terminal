#ifndef TERMPROPERTIES_H
#define TERMPROPERTIES_H

#include <QVariant>

enum TermProperty {
    WorkingDir,
    ColorScheme,
    ShellProgram
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

    QVariant &operator[](const TermProperty &key);
    const QVariant operator[](const TermProperty &key) const;

private:
    QMap<TermProperty, QVariant> m_properties;
};

#endif // TERMPROPERTIES_H
