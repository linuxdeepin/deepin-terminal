// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

TermProperties::TermProperties(QMap<TermProperty, QVariant> list) : m_properties(list)
{
}

void TermProperties::setWorkingDir(QString workingDir)
{
    m_properties.insert(WorkingDir, workingDir);
}

void TermProperties::setTermPropertyMap(QMap<TermProperty, QVariant> list)
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
