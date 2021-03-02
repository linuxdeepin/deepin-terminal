/*
    Copyright 2013 Christian Surlykke

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301  USA.
*/
#include <QApplication>
#include <QTextStream>
#include <QDebug>

#include "TerminalCharacterDecoder.h"
#include "Emulation.h"
#include "HistorySearch.h"

HistorySearch::HistorySearch(EmulationPtr emulation,
                             QString searchText,
                             bool forwards,
                             bool isLastForwards,
                             int startColumn,
                             int startLine,
                             QObject *parent) :
    QObject(parent),
    m_emulation(emulation),
    m_searchText(searchText),
    m_forwards(forwards),
    m_isLastForwards(isLastForwards),
    m_startColumn(startColumn),
    m_startLine(startLine),
    m_foundStartColumn(0),
    m_foundStartLine(0),
    m_foundEndColumn(0),
    m_foundEndLine(0),
    m_loseChinese(0),
    m_matchChinese(0),
    m_lastBackwardsPosition(-1)
{
}

HistorySearch::~HistorySearch()
{
}

void HistorySearch::search(int currBackwardsPosition, int lastFoundStartColumn, int lastFoundStartLine)
{
    bool found = false;

    m_currBackwardsPosition = currBackwardsPosition;
    m_lastFoundStartColumn = lastFoundStartColumn;
    m_lastFoundStartLine = lastFoundStartLine;

    if (!m_searchText.isEmpty()) {
        if (m_forwards) {
            found = search(m_startColumn, m_startLine, -1, m_emulation->lineCount()) || search(0, 0, m_startColumn, m_startLine);
        } else {
            found = search(0, 0, m_startColumn, m_startLine) || search(m_startColumn, m_startLine, -1, m_emulation->lineCount());
        }

        if (found) {
            emit matchFound(m_foundStartColumn, m_foundStartLine, m_foundEndColumn, m_foundEndLine, m_lastBackwardsPosition, m_loseChinese, m_matchChinese);
        } else {
            emit noMatchFound();
        }
    }

    deleteLater();
}

bool HistorySearch::search(int startColumn, int startLine, int endColumn, int endLine)
{
    qDebug() << "search from" << startColumn << "," << startLine
             <<  "to" << endColumn << "," << endLine;

    int linesRead = 0;
    int linesToRead = endLine - startLine + 1;

    qDebug() << "linesToRead:" << linesToRead;

    QString stringBackwardsAfterForwards;
    QTextStream backwardsAfterForwardsStream(&stringBackwardsAfterForwards);

    QString string;
    QTextStream searchStream(&string);
    // We read process history from (and including) startLine to (and including) endLine in
    // blocks of at most 10K lines so that we do not use unhealthy amounts of memory
    int blockSize;
    while ((blockSize = qMin(10000, linesToRead - linesRead)) > 0) {

        PlainTextDecoder decoder;
        decoder.begin(&searchStream);
        decoder.setRecordLinePositions(true);

        // Calculate lines to read and read them
        int blockStartLine = m_forwards ? startLine + linesRead : endLine - linesRead - blockSize + 1;
        int blockEndLine = blockStartLine + blockSize - 1;
        m_emulation->writeToStream(&decoder, blockStartLine, blockEndLine);

        decoder.end();

        // We search between startColumn in the first line of the string and endColumn in the last
        // line of the string. First we calculate the position (in the string) of endColumn in the
        // last line of the string
        int endPosition;

        // The String that Emulator.writeToStream produces has a newline at the end, and so ends with an
        // empty line - we ignore that.
        int numberOfLinesInString = decoder.linePositions().size() - 1;
        if (numberOfLinesInString > 0 && endColumn > -1) {
            endPosition = decoder.linePositions().at(numberOfLinesInString - 1) + endColumn;
            qDebug() << "endPosition 0:" << endPosition;
            if (!m_forwards) {
                qDebug() << "numberOfLinesInString string.size():" << string.size();
                if ((endPosition - string.size()) > 0) {
                    endPosition = string.size();
                    qDebug() << "endPosition 1:" << endPosition;
                }
                else {
                    qDebug() << "m_currBackwardsPosition:" << m_currBackwardsPosition;
                    //上次进行正向搜索，当前采用反向搜索
                    if (m_isLastForwards) {
                        PlainTextDecoder decoder;
                        decoder.begin(&backwardsAfterForwardsStream);
                        decoder.setRecordLinePositions(false);
                        int blockStartLine = 0;
                        int blockEndLine = m_lastFoundStartLine-1;
                        if (-1 == blockEndLine) {
                            blockEndLine = m_emulation->lineCount();
                        }
                        m_emulation->writeToStream(&decoder, blockStartLine, blockEndLine);

                        decoder.end();

                        endPosition = stringBackwardsAfterForwards.length() + endColumn;
                        qDebug() << "endPosition 2:" << endPosition;
                    }
                    else {
                        qDebug() << "endPosition now is:" << endPosition;
                        //endPosition值比上次搜索结果的位置还大，则重新计算endPosition
                        if (m_currBackwardsPosition > -1) {
                            if ((endPosition - m_currBackwardsPosition > m_searchText.length())) {
                                endPosition = m_currBackwardsPosition - m_searchText.length();
                                qDebug() << "endPosition > m_currBackwardsPosition 111:" << endPosition;
                            }
                            else {
                                endPosition = m_currBackwardsPosition;
                                qDebug() << "endPosition < m_currBackwardsPosition 222:" << endPosition;
                            }
                        }
                        qDebug() << "endPosition 3:" << endPosition;
                    }
                }

                //保存上次反向搜索的结束位置
                if (endPosition != -1) {
                    m_lastBackwardsPosition = endPosition;
                    qDebug() << "backwards save m_lastBackwardsPosition 1:" << m_lastBackwardsPosition;
                }
            }
        } else {
            if (m_forwards) {
                endPosition = string.size();
                qDebug() << "endPosition 4:" << endPosition;
            }
            else {
                endPosition = endColumn;
                //保存上次反向搜索的结束位置
                if (endPosition != -1) {
                    m_lastBackwardsPosition = endPosition;
                    qDebug() << "backwards save m_lastBackwardsPosition2:" << m_lastBackwardsPosition;
                }
                qDebug() << "endPosition 5!!!!!!!!!!!! == endColumn:" << endPosition;
            }
        }

        qDebug() << "At last, endPosition is:" << endPosition;

        // So now we can log for m_regExp in the string between startColumn and endPosition
        int matchStart = -1;
        if (m_forwards) {
            matchStart = string.indexOf(m_searchText, startColumn, Qt::CaseSensitive);
            qDebug() << "forwards matchStart:" << matchStart << ", startColumn:" << startColumn;
            if (matchStart >= endPosition) {
                matchStart = -1;
                qDebug() << "forwards matchStart == -1:" << matchStart;
            }
        } else {
            qDebug() << "string:" << string;
            qDebug() << "m_lastBackwardsPosition:" << m_lastBackwardsPosition;
            qDebug() << "string length:" << string.length() << ", endPosition:" << endPosition;
            //考虑了查找结果在第一行的情况
            if (0 == startLine && startLine == endLine && endPosition > endColumn) {
                endPosition = endColumn;
            }

            matchStart = string.lastIndexOf(m_searchText, endPosition - 1, Qt::CaseSensitive);
            if (matchStart != -1) {
                m_lastBackwardsPosition = matchStart;
                qDebug() << "backwards save m_lastBackwardsPosition3:" << m_lastBackwardsPosition;
            }
            qDebug() << "backwards matchStart:" << matchStart << ", startColumn:" << startColumn;
//            if (matchStart < startColumn) {
//                matchStart = -1;
//                qDebug() << "backwards matchStart == -1:" << matchStart;
//            }
        }

        if (matchStart > -1) {
            int matchEnd = matchStart + m_searchText.length() - 1;
            qDebug() << "Found in string from" << matchStart << "to" << matchEnd;

            // Translate startPos and endPos to startColum, startLine, endColumn and endLine in history.
            int startLineNumberInString = findLineNumberInString(decoder.linePositions(), matchStart);
            m_foundStartColumn = matchStart - decoder.linePositions().at(startLineNumberInString);
            m_foundStartLine = startLineNumberInString + startLine + linesRead;

            int endLineNumberInString = findLineNumberInString(decoder.linePositions(), matchEnd);
            m_foundEndColumn = matchEnd - decoder.linePositions().at(endLineNumberInString);
            m_foundEndLine = endLineNumberInString + startLine + linesRead;

            //特殊情况, 反向查找需要重新计算下endPosition并保存到m_lastBackwardsPosition
            if (!m_forwards) {
                //考虑了查找结果在第一行的情况
                if (0 == m_foundStartLine && m_foundStartLine == m_foundEndLine) {
                    m_lastBackwardsPosition = m_foundStartColumn;
                    qDebug() << "!!! ready to change m_foundStartColumn: " << m_foundStartColumn;
                    qDebug() << "!!! ready to change m_foundEndColumn: " << m_foundEndColumn;
                    qDebug() << "!!! ready to change m_foundStartLine: " << m_foundStartLine;
                    qDebug() << "!!! ready to change m_foundEndLine: " << m_foundEndLine;
                }
                else if (-1 == endPosition || -1 == endColumn) {
                    QString stringAfterSearch;
                    QTextStream afterSearchStream(&stringAfterSearch);
                    PlainTextDecoder decoder;
                    decoder.begin(&afterSearchStream);
                    decoder.setRecordLinePositions(false);
                    int blockStartLine = 0;
                    int blockEndLine = m_foundStartLine-1;
                    if (-1 == blockEndLine) {
                        blockEndLine = m_emulation->lineCount();
                    }
                    m_emulation->writeToStream(&decoder, blockStartLine, blockEndLine);

                    decoder.end();

                    m_lastBackwardsPosition = stringAfterSearch.length() + m_foundStartColumn;
                    qDebug() << "!!! ready to change m_lastBackwardsPosition: " << m_lastBackwardsPosition;
                }
            }

            //看看能不能从m_foundEndColumn m_foundEndLine和m_foundStartColumn m_foundStartLine推算出目前查找的字符串index
            //在正向查找的时候，保存当前indexOf(search, totalString) 到 m_lastBackwardsPosition
            // 总共读了多少行，记录下字符串总长度，加上当前偏移量

            qDebug() << "m_foundStartColumn" << m_foundStartColumn
                     << "m_foundStartLine" << m_foundStartLine
                     << "m_foundEndColumn" << m_foundEndColumn
                     << "m_foundEndLine" << m_foundEndLine;

            /***add begin by ut001121 zhangmeng 20200515 修复BUG22626***/
            /**
              string:   aaa-------------bbbbbbbbbbbbb-------ccc
                        |              ||           |
              match pos:|              |matchStart  matchEnd
              lose pos: loseStart      loseEnd      |
              line pos: lineStart                   lineEnd

              存在特殊情况:一个完整的物理行显示在终端被分成多个逻辑行
            */
            //中文字符正则表达式
            QRegExp regEx("[\u4E00-\u9FA5，《。》、？；：【】～！￥（）]+");

            //未匹配的串-物理行开始和结束位置
            int loseEnd = matchStart;
            int loseStart = string.lastIndexOf('\n', loseEnd) + 1;
            //qDebug() << "loseStart" << loseStart << "loseEnd" << loseEnd ;

//            if (loseStart < 0 || loseStart > loseEnd) {
//                qDebug() << "========loseStart and loseEnd is not expected" << loseStart << loseEnd;
//                exit(-1);
//            }

            //未匹配的串-物理行字符串
            QString loseStr = string.mid(loseStart, loseEnd - loseStart);
            //qDebug() << "loseStr" << loseStr << m_lineChinese;

            //未匹配的串-逻辑行字符串
            int loseEndLineNumberInString = findLineNumberInString(decoder.linePositions(), loseEnd);
            int loseEndColumn = matchStart - decoder.linePositions().at(loseEndLineNumberInString);
            QString logicLoseStr = loseStr.right(loseEndColumn);
            m_loseChinese = logicLoseStr.count(regEx);
            //qDebug() << "logicLoseStr" << logicLoseStr << m_lineChinese << loseEndColumn;

            //匹配内容是否跨多个逻辑行
            if (m_foundStartLine == m_foundEndLine) {
                /*
                 * 单逻辑行匹配情况
                 * 匹配字符的当前逻辑行:(匹配字符-当前逻辑行-开始位置)--->(匹配字符-当前逻辑行-结束位置
                */
                //匹配字符包含中文字符数量
                QString txt = m_searchText;//m_regExp.pattern();
                m_matchChinese = txt.count(regEx);
                m_matchChinese += m_loseChinese;
            } else {
                /*
                 * 多逻辑行匹配情况
                 * 匹配字符的尾行逻辑行:(匹配字符-尾行逻辑行-开始位置)--->(匹配字符-尾行逻辑行-结束位置)
                */
                QString macthStr = string.mid(loseStart, matchEnd - loseStart + 1);
                QString tailMacthStr = macthStr.right(m_foundEndColumn + 1);
                //qDebug() << "tailMacthStr" << tailMacthStr;
                m_matchChinese = tailMacthStr.count(regEx) ;
                /*
                 * 跨行匹配不计算m_loseChinese
                 * m_matchChinese += m_loseChinese;
                 */
            }
            /***add end by ut001121***/
            return true;
        }


        linesRead += blockSize;
    }

    qDebug() << "Not found";
    return false;
}


int HistorySearch::findLineNumberInString(QList<int> linePositions, int position)
{
    int lineNum = 0;
    while (lineNum + 1 < linePositions.size() && linePositions[lineNum + 1] <= position)
        lineNum++;

    return lineNum;
}
