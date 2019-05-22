#include "termproperties.h"

TermProperties::TermProperties(QString workingDir)
{
    m_properties.insert(WorkingDir, workingDir);
}

TermProperties::TermProperties(QString workingDir, QString colorScheme)
{
    m_properties.insert(WorkingDir, workingDir);
    m_properties.insert(ColorScheme, colorScheme);
}

TermProperties::TermProperties(QMap<TermProperty, QVariant> list)
{
    m_properties = list;
}

bool TermProperties::contains(TermProperty propertyType) const
{
    return m_properties.contains(propertyType);
}

QVariant &TermProperties::operator[](const TermProperty &key)
{
    return m_properties[key];
}

const QVariant TermProperties::operator[](const TermProperty &key) const
{
    return m_properties[key];
}
