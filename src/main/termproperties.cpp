// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "termproperties.h"
#include <QDebug>

TermProperties::TermProperties(QString workingDir)
{
    // qDebug() << "Enter TermProperties constructor with workingDir";
    m_properties.insert(WorkingDir, workingDir);
}

TermProperties::TermProperties(QString workingDir, QString colorScheme)
{
    // qDebug() << "Enter TermProperties constructor with workingDir and colorScheme";
    m_properties.insert(WorkingDir, workingDir);
    m_properties.insert(ColorScheme, colorScheme);
}

TermProperties::TermProperties(QMap<TermProperty, QVariant> list) : m_properties(list)
{
    // qDebug() << "Enter TermProperties constructor with property map";
}

void TermProperties::setWorkingDir(QString workingDir)
{
    // qDebug() << "Enter TermProperties::setWorkingDir";
    m_properties.insert(WorkingDir, workingDir);
}

void TermProperties::setTermPropertyMap(QMap<TermProperty, QVariant> list)
{
    // qDebug() << "Enter TermProperties::setTermPropertyMap";
    m_properties = list;
}

bool TermProperties::contains(TermProperty propertyType) const
{
    // qDebug() << "Enter TermProperties::contains";
    return m_properties.contains(propertyType);
}

QVariant &TermProperties::operator[](const TermProperty &key)
{
    // qDebug() << "Enter TermProperties::operator[] (non-const)";
    return m_properties[key];
}

const QVariant TermProperties::operator[](const TermProperty &key) const
{
    // qDebug() << "Enter TermProperties::operator[] (const)";
    return m_properties[key];
}
