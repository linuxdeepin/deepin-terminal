// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DHISTORYSEARCH_H
#define DHISTORYSEARCH_H

#include <QObject>
#include <QPointer>

class DTerminalScreen;

/*******************************************************************************
 1. @类名:    DHistorySearch
 2. @作者:    ut001213 liuzhangjian
 3. @日期:    2026-11-13
 4. @说明:    历史缓冲搜索（自研实现）。以 10K 行分块扫描屏幕
            与历史明文，支持正/反向查找与"正转反"位置换算；
            统计中文损失/匹配数（deepin 自有 BUG22626 修复移植）。
            搜索完成即自毁。
*******************************************************************************/
class DHistorySearch : public QObject
{
    Q_OBJECT
public:
    explicit DHistorySearch(DTerminalScreen *screen,
                            const QString &searchText,
                            bool forwards,
                            bool isLastForwards,
                            int startColumn,
                            int startLine,
                            QObject *parent = nullptr);

    void search(int currBackwardsPosition, int lastFoundStartColumn, int lastFoundStartLine);

signals:
    void matchFound(int startColumn, int startLine, int endColumn, int endLine,
                    int lastBackwardsPosition, int loseChinese, int matchChinese);
    void noMatchFound();

private:
    bool searchRange(int startColumn, int startLine, int endColumn, int endLine);
    static int lineNumberInPositions(const QList<int> &linePositions, int position);

    DTerminalScreen *m_screen;
    QString m_searchText;
    bool m_forwards;
    bool m_isLastForwards;
    int m_startColumn;
    int m_startLine;

    int m_foundStartColumn = 0;
    int m_foundStartLine = 0;
    int m_foundEndColumn = 0;
    int m_foundEndLine = 0;

    int m_loseChinese = 0;
    int m_matchChinese = 0;
    int m_currBackwardsPosition = -1;
    int m_lastBackwardsPosition = -1;

    int m_lastFoundStartColumn = 0;
    int m_lastFoundStartLine = 0;
};

#endif  // DHISTORYSEARCH_H
