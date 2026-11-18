// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dcolorscheme.h"

#include <QColor>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QSettings>

namespace {

// 默认配色（深底白字，标准终端色）
const DTerminalPaletteEntry kDefaultPalette[] = {
    // 前景/背景
    DTerminalPaletteEntry(QColor(0xFF, 0xFF, 0xFF), false, DTerminalPaletteEntry::WeightBold),
    DTerminalPaletteEntry(QColor(0x00, 0x00, 0x00), true),
    // 常规 8 色
    DTerminalPaletteEntry(QColor(0x00, 0x00, 0x00), false),
    DTerminalPaletteEntry(QColor(0xB2, 0x18, 0x18), false),
    DTerminalPaletteEntry(QColor(0x18, 0xB2, 0x18), false),
    DTerminalPaletteEntry(QColor(0xB2, 0x68, 0x18), false),
    DTerminalPaletteEntry(QColor(0x18, 0x18, 0xB2), false),
    DTerminalPaletteEntry(QColor(0xB2, 0x18, 0xB2), false),
    DTerminalPaletteEntry(QColor(0x18, 0xB2, 0xB2), false),
    DTerminalPaletteEntry(QColor(0xB2, 0xB2, 0xB2), false),
    // 前景/背景增强
    DTerminalPaletteEntry(QColor(0xFF, 0xFF, 0xFF), false),
    DTerminalPaletteEntry(QColor(0x00, 0x00, 0x00), false),
    // 增强 8 色
    DTerminalPaletteEntry(QColor(0x68, 0x68, 0x68), false),
    DTerminalPaletteEntry(QColor(0xFF, 0x54, 0x54), false),
    DTerminalPaletteEntry(QColor(0x54, 0xFF, 0x54), false),
    DTerminalPaletteEntry(QColor(0xFF, 0xFF, 0x54), false),
    DTerminalPaletteEntry(QColor(0x54, 0x54, 0xFF), false),
    DTerminalPaletteEntry(QColor(0xFF, 0x54, 0xFF), false),
    DTerminalPaletteEntry(QColor(0x54, 0xFF, 0xFF), false),
    DTerminalPaletteEntry(QColor(0xFF, 0xFF, 0xFF), false)
};

}  // namespace

const DTerminalPaletteEntry DColorScheme::kDefaultTable[kPaletteEntryCount] = {
    kDefaultPalette[0], kDefaultPalette[1], kDefaultPalette[2], kDefaultPalette[3],
    kDefaultPalette[4], kDefaultPalette[5], kDefaultPalette[6], kDefaultPalette[7],
    kDefaultPalette[8], kDefaultPalette[9], kDefaultPalette[10], kDefaultPalette[11],
    kDefaultPalette[12], kDefaultPalette[13], kDefaultPalette[14], kDefaultPalette[15],
    kDefaultPalette[16], kDefaultPalette[17], kDefaultPalette[18], kDefaultPalette[19]
};

const QString DColorScheme::kColorNames[kPaletteEntryCount] = {
    QStringLiteral("Foreground"), QStringLiteral("Background"),
    QStringLiteral("Color0"), QStringLiteral("Color1"), QStringLiteral("Color2"),
    QStringLiteral("Color3"), QStringLiteral("Color4"), QStringLiteral("Color5"),
    QStringLiteral("Color6"), QStringLiteral("Color7"),
    QStringLiteral("ForegroundIntense"), QStringLiteral("BackgroundIntense"),
    QStringLiteral("Color0Intense"), QStringLiteral("Color1Intense"), QStringLiteral("Color2Intense"),
    QStringLiteral("Color3Intense"), QStringLiteral("Color4Intense"), QStringLiteral("Color5Intense"),
    QStringLiteral("Color6Intense"), QStringLiteral("Color7Intense")
};

DColorScheme::DColorScheme() = default;

DColorScheme::DColorScheme(const DColorScheme &other)
    : m_opacity(other.m_opacity)
    , m_randomizeBackground(other.m_randomizeBackground)
{
    setName(other.name());
    setDescription(other.description());

    if (other.m_table != nullptr) {
        for (int i = 0; i < kPaletteEntryCount; ++i)
            setPaletteEntry(i, other.m_table[i]);
    }

    if (other.m_randomTable != nullptr) {
        for (int i = 0; i < kPaletteEntryCount; ++i) {
            const RandomRange &range = other.m_randomTable[i];
            setRandomRange(i, range.hue, range.saturation, range.value);
        }
    }
}

DColorScheme::~DColorScheme()
{
    delete[] m_table;
    delete[] m_randomTable;
}

const DTerminalPaletteEntry *DColorScheme::paletteTable() const
{
    if (m_table != nullptr)
        return m_table;
    return kDefaultTable;
}

QString DColorScheme::colorNameForIndex(int index)
{
    Q_ASSERT(index >= 0 && index < kPaletteEntryCount);
    return kColorNames[index];
}

void DColorScheme::read(const QString &fileName)
{
    QSettings settings(fileName, QSettings::IniFormat);
    settings.beginGroup(QLatin1String("General"));

    m_description = settings.value(QLatin1String("Description"), QObject::tr("Un-named Color Scheme")).toString();
    m_opacity = settings.value(QLatin1String("Opacity"), qreal(1.0)).toDouble();
    settings.endGroup();

    for (int i = 0; i < kPaletteEntryCount; ++i)
        readColorEntry(&settings, i);
}

void DColorScheme::readColorEntry(QSettings *settings, int index)
{
    const QString colorName = colorNameForIndex(index);
    settings->beginGroup(colorName);

    DTerminalPaletteEntry entry;

    // 颜色值：逗号分隔的 r,g,b（QSettings 可能解析为字符串或字符串列表）
    const QVariant colorValue = settings->value(QLatin1String("Color"));
    QString colorString;
    int r = 0, g = 0, b = 0;
    bool ok = false;
    if (colorValue.type() == QVariant::StringList) {
        const QStringList rgbList = colorValue.toStringList();
        colorString = rgbList.join(QLatin1Char(','));
        if (rgbList.count() == 3) {
            bool parseOk = false;
            ok = true;
            r = rgbList[0].toInt(&parseOk);
            ok = ok && parseOk && r >= 0 && r <= 0xff;
            g = rgbList[1].toInt(&parseOk);
            ok = ok && parseOk && g >= 0 && g <= 0xff;
            b = rgbList[2].toInt(&parseOk);
            ok = ok && parseOk && b >= 0 && b <= 0xff;
        }
    } else if (colorValue.type() == QVariant::String) {
        colorString = colorValue.toString();
        const QStringList rgbList = colorString.split(QLatin1Char(','));
        if (rgbList.count() == 3) {
            bool parseOk = false;
            ok = true;
            r = rgbList[0].toInt(&parseOk);
            ok = ok && parseOk && r >= 0 && r <= 0xff;
            g = rgbList[1].toInt(&parseOk);
            ok = ok && parseOk && g >= 0 && g <= 0xff;
            b = rgbList[2].toInt(&parseOk);
            ok = ok && parseOk && b >= 0 && b <= 0xff;
        }
    }

    if (ok)
        entry.color = QColor(r, g, b);
    else {
        // 支持 HTML 风格色值（#RRGGBB）
        const QColor namedColor(colorString);
        if (namedColor.isValid())
            entry.color = namedColor;
    }

    // 透明与字重（背景默认透明、前景默认粗体）
    entry.transparent = settings->value(QLatin1String("Transparent"), false).toBool();
    if (index == 1 /*Background*/)
        entry.transparent = settings->value(QLatin1String("Transparent"), true).toBool();

    const bool bold = settings->value(QLatin1String("Bold"), false).toBool();
    if (index == 0 /*Foreground*/)
        entry.weight = bold ? DTerminalPaletteEntry::WeightBold : DTerminalPaletteEntry::WeightNormal;
    else
        entry.weight = DTerminalPaletteEntry::WeightAuto;

    // 背景随机化范围
    const QVariant randomValue = settings->value(QLatin1String("RandomizationRange"));
    if (randomValue.type() == QVariant::StringList) {
        const QStringList range = randomValue.toStringList();
        if (range.count() == 3) {
            const int hue = range[0].toInt();
            const int saturation = range[1].toInt();
            const int value = range[2].toInt();
            if (hue >= 0 && hue <= kMaxHue
                    && saturation >= 0 && saturation <= 255
                    && value >= 0 && value <= 255) {
                setRandomRange(index, static_cast<quint16>(hue),
                               static_cast<quint8>(saturation), static_cast<quint8>(value));
                m_randomizeBackground = true;
            }
        }
    }

    settings->endGroup();

    // 读取成功则落地
    setPaletteEntry(index, entry);
}

void DColorScheme::setPaletteEntry(int index, const DTerminalPaletteEntry &entry)
{
    Q_ASSERT(index >= 0 && index < kPaletteEntryCount);

    if (m_table == nullptr) {
        m_table = new DTerminalPaletteEntry[kPaletteEntryCount];
        for (int i = 0; i < kPaletteEntryCount; ++i)
            m_table[i] = kDefaultTable[i];
    }
    m_table[index] = entry;
}

void DColorScheme::setRandomRange(int index, quint16 hue, quint8 saturation, quint8 value)
{
    if (m_randomTable == nullptr)
        m_randomTable = new RandomRange[kPaletteEntryCount];

    m_randomTable[index].hue = hue;
    m_randomTable[index].saturation = saturation;
    m_randomTable[index].value = value;
}

void DColorScheme::fillPalette(DTerminalPaletteEntry *table, uint randomSeed) const
{
    for (int i = 0; i < kPaletteEntryCount; ++i)
        table[i] = paletteEntry(i, randomSeed);
}

DTerminalPaletteEntry DColorScheme::paletteEntry(int index, uint randomSeed) const
{
    Q_ASSERT(index >= 0 && index < kPaletteEntryCount);

    DTerminalPaletteEntry entry = paletteTable()[index];

    if (randomSeed != 0 && m_randomTable != nullptr && !m_randomTable[index].isNull()) {
        const RandomRange &range = m_randomTable[index];

        // 基于原色做 HSV 偏移（随机化的是偏移量，保持可读性）
        const int hueDifference = range.hue ? (rand() % range.hue) - range.hue / 2 : 0;
        const int saturationDifference = range.saturation ? (rand() % range.saturation) - range.saturation / 2 : 0;
        const int valueDifference = range.value ? (rand() % range.value) - range.value / 2 : 0;

        QColor color = entry.color;
        const int newHue = qAbs((color.hue() + hueDifference) % 360);
        const int newSaturation = qMin(qAbs(color.saturation() + saturationDifference), 255);
        const int newValue = qMin(qAbs(color.value() + valueDifference), 255);
        color.setHsv(newHue, newSaturation, newValue);
        entry.color = color;
    }

    return entry;
}

QColor DColorScheme::foregroundColor() const
{
    return paletteEntry(0).color;
}

QColor DColorScheme::backgroundColor() const
{
    return paletteEntry(1).color;
}

bool DColorScheme::hasDarkBackground() const
{
    return backgroundColor().value() < 127;
}

// ---- DColorSchemeRegistry ----

DColorSchemeRegistry::DColorSchemeRegistry() = default;

DColorSchemeRegistry::~DColorSchemeRegistry()
{
    qDeleteAll(m_modified);
}

DColorSchemeRegistry *DColorSchemeRegistry::instance()
{
    static DColorSchemeRegistry registry;
    return &registry;
}

const DColorScheme *DColorSchemeRegistry::defaultScheme() const
{
    static const DColorScheme defaultScheme;
    return &defaultScheme;
}

void DColorSchemeRegistry::loadAllSchemes()
{
    const QStringList paths = listSchemePaths();
    for (const QString &path : paths)
        loadScheme(path);
    m_loadedAll = true;
}

QStringList DColorSchemeRegistry::listSchemePaths()
{
    QStringList paths;

    // 数据目录：Debug 构建（RUNDIR_KEYBOARD_LAYOUT_FIRST）优先读二进制旁的
    // color-schemes/（与 keytab 同机制），找不到再回退安装路径 COLORSCHEMES_DIR。
    QStringList dataDirs;
#if defined(RUNDIR_KEYBOARD_LAYOUT_FIRST)
    const QDir appDir(QCoreApplication::applicationDirPath());
    if (appDir.exists(QStringLiteral("color-schemes")))
        dataDirs << appDir.absoluteFilePath(QStringLiteral("color-schemes"));
#endif
    dataDirs << QStringLiteral(COLORSCHEMES_DIR);
    for (const QString &dataDir : dataDirs) {
        const QDir dir(dataDir);
        if (!dir.exists())
            continue;
        const QStringList entries = dir.entryList(QStringList() << QStringLiteral("*.colorscheme"),
                                                  QDir::Files);
        for (const QString &entry : entries)
            paths << dir.absoluteFilePath(entry);
    }

    // 自定义目录
    for (const QString &customDir : m_customDirs) {
        const QDir dir(customDir);
        if (!dir.exists())
            continue;
        const QStringList entries = dir.entryList(QStringList() << QStringLiteral("*.colorscheme"),
                                                  QDir::Files);
        for (const QString &entry : entries)
            paths << dir.absoluteFilePath(entry);
    }

    return paths;
}

QString DColorSchemeRegistry::findSchemePath(const QString &name)
{
    const QStringList paths = listSchemePaths();
    for (const QString &path : paths) {
        if (QFileInfo(path).baseName() == name)
            return path;
    }
    return QString();
}

bool DColorSchemeRegistry::loadScheme(const QString &path)
{
    if (path.isEmpty())
        return false;

    QFileInfo info(path);
    const QString schemeName = info.baseName();
    DColorScheme *scheme = new DColorScheme();
    scheme->setName(schemeName);
    scheme->read(path);

    if (!m_schemes.contains(schemeName))
        m_modified.insert(scheme);
    m_schemes.insert(schemeName, scheme);
    return true;
}

const DColorScheme *DColorSchemeRegistry::findScheme(const QString &name)
{
    if (name.isEmpty())
        return defaultScheme();

    if (!m_loadedAll)
        loadAllSchemes();

    return m_schemes.value(name, nullptr);
}

QList<const DColorScheme *> DColorSchemeRegistry::allSchemes()
{
    if (!m_loadedAll)
        loadAllSchemes();
    return m_schemes.values();
}

void DColorSchemeRegistry::reloadScheme(const QString &origName)
{
    // 按路径重新加载主题
    const QString path = findSchemePath(origName);
    if (path.isEmpty())
        return;

    const QString schemeName = QFileInfo(path).baseName();
    DColorScheme *scheme = new DColorScheme();
    scheme->setName(schemeName);
    scheme->read(path);

    delete m_schemes.value(schemeName, nullptr);
    m_schemes.insert(schemeName, scheme);
    m_modified.insert(scheme);
}

bool DColorSchemeRegistry::loadCustomScheme(const QString &path)
{
    return loadScheme(path);
}

void DColorSchemeRegistry::addCustomDir(const QString &customDir)
{
    if (!m_customDirs.contains(customDir))
        m_customDirs.append(customDir);
}
