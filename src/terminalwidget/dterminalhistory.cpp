// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dterminalhistory.h"

#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QTemporaryFile>

#include <cstdio>

namespace {

// ---- 临时日志文件：顺序读写 + 尾部截断（历史文件的底层存储单元） ----

class DLogFile
{
public:
    DLogFile()
    {
        // 直接使用标准缓存目录，避免失效的自定义路径回退链
        QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
        if (!QDir().mkpath(cacheDir)) {
            qWarning() << "DLogFile: cannot create cache directory" << cacheDir;
        }
        m_file.setFileTemplate(cacheDir + QLatin1Char('/') + QLatin1String("dterminal-XXXXXX.history"));
        m_file.open();
    }

    qint64 length() const { return m_length; }

    void append(const char *buffer, qint64 count)
    {
        if (!m_file.seek(m_length) || m_file.write(buffer, count) < 0) {
            std::perror("DLogFile::append");
            return;
        }
        m_length += count;
    }

    void readAt(char *buffer, qint64 size, qint64 location) const
    {
        if (location < 0 || size < 0 || location + size > m_length) {
            std::fprintf(stderr, "DLogFile::readAt(%lld,%lld): invalid args.\n", size, location);
            return;
        }
        if (!m_file.seek(location)) {
            std::perror("DLogFile::readAt.seek");
            return;
        }
        if (m_file.read(buffer, size) < 0) {
            std::perror("DLogFile::readAt.read");
            return;
        }
    }

    void truncate(qint64 location)
    {
        if (location < 0 || location > m_length) {
            std::fprintf(stderr, "DLogFile::truncate(%lld): invalid args.\n", location);
            return;
        }
        m_length = location;
    }

private:
    mutable QTemporaryFile m_file;
    qint64 m_length = 0;
};

// ---- 内存紧凑历史 ----

class DTerminalHistoryBuffer final : public DTerminalHistory
{
public:
    explicit DTerminalHistoryBuffer(int maxLines)
    {
        m_maxLines = qMax(0, maxLines);
    }

    int maxLineCount() const override { return m_maxLines; }
    int lineCount() const override { return m_lineEnds.size(); }
    int lineLength(int line) const override
    {
        if (line < 0 || line >= m_lineEnds.size())
            return 0;
        return line == 0 ? m_lineEnds[0] : m_lineEnds[line] - m_lineEnds[line - 1];
    }

    void lineCells(int line, int startColumn, int count, DTerminalChar *out) const override
    {
        if (count == 0)
            return;
        Q_ASSERT(line < m_lineEnds.size());
        Q_ASSERT(startColumn >= 0);
        Q_ASSERT(startColumn <= lineLength(line) - count);

        const int base = startOfLine(line) + startColumn;
        std::copy(m_cells.constBegin() + base, m_cells.constBegin() + base + count, out);
    }

    bool isWrappedLine(int line) const override
    {
        Q_ASSERT(line < m_lineEnds.size());
        return m_flags[line] & LineWrapped;
    }

    quint8 lineFlags(int line) const override
    {
        Q_ASSERT(line < m_lineEnds.size());
        return m_flags[line];
    }

    void appendCells(const DTerminalChar *cells, int count) override
    {
        const int oldSize = m_cells.size();
        m_cells.resize(oldSize + count);
        std::copy(cells, cells + count, m_cells.begin() + oldSize);
        m_lineEnds.append(m_cells.size());
        m_flags.append(LineDefault);

        while (m_lineEnds.size() > m_maxLines)
            dropFirstLine();
    }

    void appendLine(quint8 flags) override
    {
        if (m_flags.isEmpty())
            return;
        m_flags.last() = flags;
    }

    void dropLastLine() override
    {
        if (m_lineEnds.size() > 1) {
            m_lineEnds.pop_back();
            m_flags.pop_back();
            m_cells.resize(m_lineEnds.last());
        } else {
            m_cells.clear();
            m_lineEnds.clear();
            m_flags.clear();
        }
    }

    int reflow(int columns) override
    {
        QVector<int> newEnds;
        QVector<quint8> newFlags;

        int current = 0;
        const int total = lineCount();
        while (current < total) {
            int startLine = startOfLine(current);
            int endLine = current + 1 < total ? startOfLine(current + 1) : m_cells.size();
            const quint8 flags = lineFlags(current);

            // 合并被软换行标记串起来的行
            while (isWrappedLine(current)) {
                current++;
                endLine = current + 1 < total ? startOfLine(current + 1) : m_cells.size();
            }

            // 按新宽度切分；双高行不参与切分
            while (endLine - startLine > columns
                   && !(flags & (LineDoubleHeightTop | LineDoubleHeightBottom))) {
                startLine += columns;
                newEnds.append(startLine);
                newFlags.append(flags | LineWrapped);
            }
            newEnds.append(endLine);
            newFlags.append(flags & ~LineWrapped);
            current++;
        }

        m_lineEnds = newEnds;
        m_flags = newFlags;

        int removed = 0;
        while (lineCount() > m_maxLines) {
            dropFirstLine();
            ++removed;
        }
        return removed;
    }

private:
    int startOfLine(int line) const
    {
        return line == 0 ? 0 : m_lineEnds[line - 1];
    }

    void dropFirstLine()
    {
        if (m_lineEnds.isEmpty())
            return;
        if (m_lineEnds.size() == 1) {
            m_cells.clear();
            m_lineEnds.clear();
            m_flags.clear();
            return;
        }
        m_flags.pop_front();
        const int removed = m_lineEnds.takeFirst();
        for (int &end : m_lineEnds)
            end -= removed;
        while (m_cells.size() > m_lineEnds.last())
            m_cells.pop_front();
    }

    QVector<DTerminalChar> m_cells;
    QVector<int> m_lineEnds;
    QVector<quint8> m_flags;
    int m_maxLines = 1000;
};

// ---- 临时文件流式历史 ----

class DTerminalHistoryFile final : public DTerminalHistory
{
public:
    int maxLineCount() const override { return lineCount(); }
    int lineCount() const override { return int(m_index.length() / sizeof(qint64)); }
    int lineLength(int line) const override
    {
        return int((startOfLine(line + 1) - startOfLine(line)) / sizeof(DTerminalChar));
    }

    void lineCells(int line, int startColumn, int count, DTerminalChar *out) const override
    {
        m_cells.readAt(reinterpret_cast<char *>(out), count * qint64(sizeof(DTerminalChar)),
                       startOfLine(line) + startColumn * qint64(sizeof(DTerminalChar)));
    }

    bool isWrappedLine(int line) const override
    {
        // 文件存储按"标志字节非零"判定软换行
        if (line >= 0 && line <= lineCount()) {
            quint8 flag = 0;
            m_flags.readAt(reinterpret_cast<char *>(&flag), 1, line);
            return flag != 0;
        }
        return false;
    }

    quint8 lineFlags(int line) const override
    {
        if (line >= 0 && line <= lineCount()) {
            quint8 flag = 0;
            m_flags.readAt(reinterpret_cast<char *>(&flag), 1, line);
            return flag;
        }
        return 0;
    }

    void appendCells(const DTerminalChar *cells, int count) override
    {
        m_cells.append(reinterpret_cast<const char *>(cells), count * qint64(sizeof(DTerminalChar)));
    }

    void appendLine(quint8 flags) override
    {
        const qint64 end = m_cells.length();
        m_index.append(reinterpret_cast<const char *>(&end), sizeof(qint64));
        m_flags.append(reinterpret_cast<const char *>(&flags), 1);
    }

    void dropLastLine() override
    {
        const int lines = lineCount();
        if (lines < 2) {
            m_cells.truncate(0);
        } else {
            m_cells.truncate(startOfLine(lines - 2));
        }
        m_index.truncate(qMax(0, lines - 1) * qint64(sizeof(qint64)));
        m_flags.truncate(qMax(0, lines - 1));
    }

    int reflow(int columns) override
    {
        const int total = lineCount();
        int current = 0;
        if (total > kReflowLineCap)
            current = total - kReflowLineCap;

        QVector<qint64> newIndex;
        QVector<quint8> newFlags;

        while (current < total) {
            qint64 startLine = startOfLine(current);
            qint64 endLine = startOfLine(current + 1);

            // 合并被软换行标记串起来的行
            while (isWrappedLine(current)) {
                current++;
                endLine = startOfLine(current + 1);
            }

            // 按新宽度切分（文件存储不识别双高行）
            while ((endLine - startLine) / qint64(sizeof(DTerminalChar)) > columns) {
                startLine += qint64(columns) * sizeof(DTerminalChar);
                newIndex.append(startLine);
                newFlags.append(LineWrapped);
            }
            newIndex.append(endLine);
            newFlags.append(LineDefault);
            current++;
        }

        // 重写索引与标志流
        m_index.truncate(0);
        m_flags.truncate(0);
        for (int i = 0; i < newIndex.size(); ++i) {
            m_index.append(reinterpret_cast<const char *>(&newIndex[i]), sizeof(qint64));
            m_flags.append(reinterpret_cast<const char *>(&newFlags[i]), 1);
        }
        return 0;
    }

private:
    qint64 startOfLine(int line) const
    {
        if (line <= 0)
            return 0;
        if (line <= lineCount()) {
            qint64 end = 0;
            m_index.readAt(reinterpret_cast<char *>(&end), sizeof(qint64), (line - 1) * qint64(sizeof(qint64)));
            return end;
        }
        return m_cells.length();
    }

    DLogFile m_cells;
    DLogFile m_index;
    DLogFile m_flags;
};

// ---- 无历史 ----

class DTerminalHistoryNone final : public DTerminalHistory
{
public:
    bool hasScrolledLines() const override { return false; }
    int maxLineCount() const override { return 0; }
    int lineCount() const override { return 0; }
    int lineLength(int) const override { return 0; }
    void lineCells(int, int, int, DTerminalChar *) const override {}
    bool isWrappedLine(int) const override { return false; }
    quint8 lineFlags(int) const override { return 0; }
    void appendCells(const DTerminalChar *, int) override {}
    void appendLine(quint8) override {}
    void dropLastLine() override {}
    int reflow(int) override { return 0; }
};

}  // namespace

DTerminalHistory::~DTerminalHistory() = default;

DTerminalHistory *DTerminalHistory::create(StorageKind kind, int maxLines)
{
    switch (kind) {
    case StorageKind::None:
        return new DTerminalHistoryNone();
    case StorageKind::Buffer:
        return new DTerminalHistoryBuffer(maxLines);
    case StorageKind::File:
        return new DTerminalHistoryFile();
    }
    return new DTerminalHistoryNone();
}

void DTerminalHistory::migrate(const DTerminalHistory *from, DTerminalHistory *to)
{
    if (from == nullptr)
        return;

    const int lines = from->lineCount();
    QVector<DTerminalChar> buffer;
    for (int i = 0; i < lines; ++i) {
        const int length = from->lineLength(i);
        buffer.resize(length);
        from->lineCells(i, 0, length, buffer.data());
        to->appendCells(buffer.constData(), length);
        to->appendLine(from->lineFlags(i));
    }
}
