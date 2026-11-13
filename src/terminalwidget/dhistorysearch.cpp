// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dhistorysearch.h"

#include "dterminalscreen.h"

#include <QDebug>
#include <QRegularExpression>

DHistorySearch::DHistorySearch(DTerminalScreen *screen,
                               const QString &searchText,
                               bool forwards,
                               bool isLastForwards,
                               int startColumn,
                               int startLine,
                               QObject *parent)
    : QObject(parent)
    , m_screen(screen)
    , m_searchText(searchText)
    , m_forwards(forwards)
    , m_isLastForwards(isLastForwards)
    , m_startColumn(startColumn)
    , m_startLine(startLine)
{
}

void DHistorySearch::search(int currBackwardsPosition, int lastFoundStartColumn, int lastFoundStartLine)
{
    bool found = false;

    m_currBackwardsPosition = currBackwardsPosition;
    m_lastFoundStartColumn = lastFoundStartColumn;
    m_lastFoundStartLine = lastFoundStartLine;

    if (!m_searchText.isEmpty()) {
        if (m_forwards) {
            found = searchRange(m_startColumn, m_startLine, -1, m_screen->lineCount())
                    || searchRange(0, 0, m_startColumn, m_startLine);
        } else {
            found = searchRange(0, 0, m_startColumn, m_startLine)
                    || searchRange(m_startColumn, m_startLine, -1, m_screen->lineCount());
        }

        if (found) {
            emit matchFound(m_foundStartColumn, m_foundStartLine,
                            m_foundEndColumn, m_foundEndLine,
                            m_lastBackwardsPosition, m_loseChinese, m_matchChinese);
        } else {
            emit noMatchFound();
        }
    }

    deleteLater();
}

int DHistorySearch::lineNumberInPositions(const QList<int> &linePositions, int position)
{
    int lineNum = 0;
    while (lineNum + 1 < linePositions.size() && linePositions[lineNum + 1] <= position)
        lineNum++;
    return lineNum;
}

bool DHistorySearch::searchRange(int startColumn, int startLine, int endColumn, int endLine)
{
    qDebug() << "search from" << startColumn << "," << startLine
             << "to" << endColumn << "," << endLine;

    int linesRead = 0;
    const int linesToRead = endLine - startLine + 1;

    QString backwardsContext;
    QString searchString;

    // 以最多 10K 行的块扫描，避免内存失控
    int blockSize = 0;
    while ((blockSize = qMin(10000, linesToRead - linesRead)) > 0) {
        QList<int> linePositions;
        const int blockStartLine = m_forwards ? startLine + linesRead
                                              : endLine - linesRead - blockSize + 1;
        const int blockEndLine = blockStartLine + blockSize - 1;
        searchString = m_screen->extractPlainText(blockStartLine, blockEndLine, &linePositions);

        // 搜索区间：首行从 startColumn 起，末行到 endColumn 止
        int endPosition = 0;

        // 提取的文本末尾带换行，最后一个空行忽略
        const int numberOfLinesInString = linePositions.size() - 1;
        if (numberOfLinesInString > 0 && endColumn > -1) {
            endPosition = linePositions.at(numberOfLinesInString - 1) + endColumn;
            if (!m_forwards) {
                if ((endPosition - searchString.size()) > 0) {
                    endPosition = searchString.size();
                } else if (m_isLastForwards) {
                    // 上次正向、本次反向：从上次结果处回溯
                    int backStart = 0;
                    int backEnd = m_lastFoundStartLine - 1;
                    if (backEnd == -1)
                        backEnd = m_screen->lineCount();
                    backwardsContext = m_screen->extractPlainText(backStart, backEnd);
                    endPosition = backwardsContext.length() + endColumn;
                } else {
                    if (m_currBackwardsPosition > -1) {
                        if ((endPosition - m_currBackwardsPosition > m_searchText.length()))
                            endPosition = m_currBackwardsPosition - m_searchText.length();
                        else
                            endPosition = m_currBackwardsPosition;
                    }
                }

                if (endPosition != -1)
                    m_lastBackwardsPosition = endPosition;
            }
        } else {
            if (m_forwards) {
                endPosition = searchString.size();
            } else {
                endPosition = endColumn;
                if (endPosition != -1)
                    m_lastBackwardsPosition = endPosition;
            }
        }

        int matchStart = -1;
        if (m_forwards) {
            matchStart = searchString.indexOf(m_searchText, startColumn, Qt::CaseSensitive);
            if (matchStart >= endPosition)
                matchStart = -1;
        } else {
            // 查找结果落在首行的情形
            if (startLine == 0 && startLine == endLine && endPosition > endColumn)
                endPosition = endColumn;

            matchStart = searchString.lastIndexOf(m_searchText, endPosition - 1, Qt::CaseSensitive);
            if (matchStart != -1)
                m_lastBackwardsPosition = matchStart;
        }

        if (matchStart > -1) {
            const int matchEnd = matchStart + m_searchText.length() - 1;

            // 字符串位置 → 行列
            const int startLineNumberInString = lineNumberInPositions(linePositions, matchStart);
            m_foundStartColumn = matchStart - linePositions.at(startLineNumberInString);
            m_foundStartLine = startLineNumberInString + startLine + linesRead;

            const int endLineNumberInString = lineNumberInPositions(linePositions, matchEnd);
            m_foundEndColumn = matchEnd - linePositions.at(endLineNumberInString);
            m_foundEndLine = endLineNumberInString + startLine + linesRead;

            // 反向查找的特殊位置换算
            if (!m_forwards) {
                if (m_foundStartLine == 0 && m_foundStartLine == m_foundEndLine) {
                    m_lastBackwardsPosition = m_foundStartColumn;
                } else if (endPosition == -1 || endColumn == -1) {
                    int backStart = 0;
                    int backEnd = m_foundStartLine - 1;
                    if (backEnd == -1)
                        backEnd = m_screen->lineCount();
                    const QString context = m_screen->extractPlainText(backStart, backEnd);
                    m_lastBackwardsPosition = context.length() + m_foundStartColumn;
                }
            }

            // 中文字符统计（deepin 自有 BUG22626 修复移植）：
            // 物理行被折成多个逻辑行时，需要统计未匹配段的中文字符
            static const QRegularExpression chinesePattern(
                QStringLiteral("[\u4E00-\u9FA5，《。》、？；：【】～！￥（）]+"));

            const int loseEnd = matchStart;
            const int loseStart = searchString.lastIndexOf(QLatin1Char('\n'), loseEnd) + 1;
            const QString loseString = searchString.mid(loseStart, loseEnd - loseStart);

            const int loseEndLineNumberInString = lineNumberInPositions(linePositions, loseEnd);
            const int loseEndColumn = matchStart - linePositions.at(loseEndLineNumberInString);
            const QString logicalLoseString = loseString.right(loseEndColumn);
            m_loseChinese = logicalLoseString.count(chinesePattern);

            if (m_foundStartLine == m_foundEndLine) {
                // 单逻辑行：匹配串 + 未匹配段的中文
                m_matchChinese = m_searchText.count(chinesePattern) + m_loseChinese;
            } else {
                // 跨逻辑行：只统计尾行匹配段的中文
                const QString matchString = searchString.mid(loseStart, matchEnd - loseStart + 1);
                const QString tailMatchString = matchString.right(m_foundEndColumn + 1);
                m_matchChinese = tailMatchString.count(chinesePattern);
            }

            return true;
        }

        linesRead += blockSize;
    }

    qInfo() << "History Search Not found";
    return false;
}
