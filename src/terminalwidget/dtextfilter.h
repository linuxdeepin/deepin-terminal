// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DTEXTFILTER_H
#define DTEXTFILTER_H

#include "dterminalutil.h"

#include <QObject>
#include <QMultiHash>
#include <QRegularExpression>
#include <QStringList>
#include <QVector>

#include <functional>

class QAction;
class QUrl;
class DTerminalScreen;

/*******************************************************************************
 1. @类名:    DTextFilter
 2. @作者:    ut001213 liuzhangjian
 3. @日期:    2026-11-13
 4. @说明:    终端文本过滤器（自研整合版）。把历史实现中
            "过滤器 + 正则过滤器 + 过滤链" 的层级合并，
            负责在终端文本缓冲上标记热点（链接/标记区）。
*******************************************************************************/
class DTextFilter : public QObject
{
    Q_OBJECT
public:
    /** 文本热点区域（自研命名 DTextSpot） */
    class Spot
    {
    public:
        enum Kind {
            KindUnspecified,
            KindLink,
            KindMarker
        };

        Spot(int startLine, int startColumn, int endLine, int endColumn);
        virtual ~Spot();

        int startLine() const { return m_startLine; }
        int startColumn() const { return m_startColumn; }
        int endLine() const { return m_endLine; }
        int endColumn() const { return m_endColumn; }
        Kind kind() const { return m_kind; }

        virtual void activate(const QString &action = QString()) = 0;
        virtual QList<QAction *> actions();

    protected:
        void setKind(Kind kind) { m_kind = kind; }

    private:
        int m_startLine;
        int m_startColumn;
        int m_endLine;
        int m_endColumn;
        Kind m_kind = KindUnspecified;
    };

    virtual ~DTextFilter();

    /** 清空内部缓冲与全部热点 */
    void reset();

    /** 设置待处理的文本缓冲与行首位置表 */
    void setBuffer(const QString *buffer, const QVector<int> *lineStarts);

    /** 对内部缓冲执行匹配 */
    virtual void process() = 0;

    Spot *spotAt(int line, int column) const;
    QList<Spot *> spots() const;
    QList<Spot *> spotsAtLine(int line) const;

protected:
    void addSpot(Spot *spot);
    const QString *buffer() const { return m_buffer; }
    /** 把缓冲内的字符位置换算为行/列（列为显示宽度） */
    void locateInBuffer(int position, int &line, int &column) const;

private:
    QMultiHash<int, Spot *> m_spotsByLine;
    QList<Spot *> m_spots;
    const QVector<int> *m_lineStarts = nullptr;
    const QString *m_buffer = nullptr;
};

/*******************************************************************************
 2. @类名:    DRegexFilter
 3. @说明:    按正则表达式标记热点（链接与标记区的基础匹配器）。
*******************************************************************************/
class DRegexFilter : public DTextFilter
{
public:
    class Spot : public DTextFilter::Spot
    {
    public:
        Spot(int startLine, int startColumn, int endLine, int endColumn);
        void activate(const QString &action = QString()) override;

        void setCapturedTexts(const QStringList &texts) { m_capturedTexts = texts; }
        QStringList capturedTexts() const { return m_capturedTexts; }

    private:
        QStringList m_capturedTexts;
    };

    void setPattern(const QRegularExpression &pattern) { m_pattern = pattern; }
    QRegularExpression pattern() const { return m_pattern; }

    void process() override;

protected:
    virtual Spot *createSpot(int startLine, int startColumn, int endLine, int endColumn);

private:
    QRegularExpression m_pattern;
};

/*******************************************************************************
 3. @类名:    DUrlSpot
 4. @说明:    URL/邮件热点。提供打开/复制动作，触发时发出 activated。
            （顶层类：moc 不支持嵌套 QObject 类）
*******************************************************************************/
class DUrlSpot : public QObject, public DRegexFilter::Spot
{
    Q_OBJECT
public:
    DUrlSpot(int startLine, int startColumn, int endLine, int endColumn);
    ~DUrlSpot() override;

    QList<QAction *> actions() override;
    void activate(const QString &action = QString()) override;

signals:
    void activated(const QUrl &url, bool fromContextMenu);

private:
    enum class UrlKind { Standard, Email, Unknown };
    UrlKind urlKind() const;
};

/*******************************************************************************
 4. @类名:    DUrlFilter
 5. @说明:    识别文本中的 URL 与邮件地址，点击/菜单触发时透传
            DUrlSpot::activated 信号。
*******************************************************************************/
class DUrlFilter : public DRegexFilter
{
    Q_OBJECT
public:
    DUrlFilter();

protected:
    DRegexFilter::Spot *createSpot(int startLine, int startColumn, int endLine, int endColumn) override;

signals:
    void activated(const QUrl &url, bool fromContextMenu);
};

/*******************************************************************************
 5. @类名:    DTextFilterChain
 6. @说明:    过滤器集合：统一 reset/process/查询热点，拥有其成员。
*******************************************************************************/
class DTextFilterChain
{
public:
    ~DTextFilterChain();

    void addFilter(DTextFilter *filter) { m_filters.append(filter); }
    void removeFilter(DTextFilter *filter);
    bool containsFilter(DTextFilter *filter) const { return m_filters.contains(filter); }
    void clear();

    void reset();
    void process();
    void setBuffer(const QString *buffer, const QVector<int> *lineStarts);

    DTextFilter::Spot *spotAt(int line, int column) const;
    QList<DTextFilter::Spot *> spots() const;

private:
    QList<DTextFilter *> m_filters;
};

/*******************************************************************************
 7. @类名:    DShellContextSink
 8. @说明:    shell 提示符/当前命令的存取接口。终端图像扫描器把识别
            结果交给该接口，M4 由会话管理器实现。
*******************************************************************************/
class DShellContextSink
{
public:
    virtual ~DShellContextSink() = default;

    virtual void saveShellPrompt(int sessionId, const QString &prompt) = 0;
    virtual QString currentShellPrompt(int sessionId) const = 0;
    virtual void saveShellCommand(int sessionId, const QString &command) = 0;
};

/*******************************************************************************
 9. @类名:    DTerminalTextScanner
 10. @说明:   终端图像文本扫描器。把当前屏幕内容展开为文本缓冲，
            驱动过滤链标记热点，并提取 shell 提示符与当前命令。
*******************************************************************************/
class DTerminalTextScanner
{
public:
    DTerminalTextScanner();
    ~DTerminalTextScanner();

    void addFilter(DTextFilter *filter);
    void setSessionId(int sessionId) { m_sessionId = sessionId; }
    void setContextSink(DShellContextSink *sink) { m_sink = sink; }

    /** 组合字符序列解析器：M3 字符解码器落地后由上层注入 */
    void setSequenceResolver(const std::function<QString(uint)> &resolver) { m_sequenceResolver = resolver; }

    /** 以当前终端图像重建文本缓冲并执行全部过滤器 */
    void scanImage(const DTerminalChar *image, int lines, int columns, const QVector<quint8> &lineFlags);

    DTextFilter::Spot *spotAt(int line, int column) const;
    QList<DTextFilter::Spot *> spots() const;

private:
    QString buildLineText(const DTerminalChar *lineCells, int columns) const;

    DTextFilterChain m_chain;
    QString m_buffer;
    QVector<int> m_lineStarts;
    int m_sessionId = 0;
    DShellContextSink *m_sink = nullptr;
    std::function<QString(uint)> m_sequenceResolver;
};

/*******************************************************************************
 11. @类名:    DUrlExtractor
 12. @说明:    OSC8 超链接转义序列提取器。跟踪"开始/结束/中止"状态，
            把链接地址、显示文本与屏幕坐标记入历史，
            供渲染层绘制下划线热区。
*******************************************************************************/
class DUrlExtractor
{
public:
    struct ExtractedUrl {
        QString url;
        QString text;
        int beginRow = 0;
        int beginColumn = 0;
        int endRow = 0;
        int endColumn = 0;
    };

    DUrlExtractor() = default;

    void setAllowedSchemas(const QStringList &schemas) { m_allowedSchemas = schemas; }
    void setScreen(DTerminalScreen *screen) { m_screen = screen; }

    bool reading() const { return m_reading; }
    void beginUrlInput() { m_reading = true; }
    void endUrlInput();
    void abortUrlInput();
    void appendUrlText(QChar c);
    void setUrl(const QString &url);
    void toggleUrlInput();

    QVector<ExtractedUrl> history() const { return m_history; }
    void clear() { m_history.clear(); }
    void historyLinesRemoved(int lines);

private:
    bool m_reading = false;
    bool m_ignoreNextToggle = false;
    ExtractedUrl m_current;
    QVector<ExtractedUrl> m_history;
    QStringList m_allowedSchemas;
    DTerminalScreen *m_screen = nullptr;
};

#endif  // DTEXTFILTER_H
