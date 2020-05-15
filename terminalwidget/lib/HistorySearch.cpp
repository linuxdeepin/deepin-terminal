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

HistorySearch::HistorySearch(EmulationPtr emulation, QRegExp regExp,
                             bool forwards, int startColumn, int startLine,
                             QObject *parent) :
    QObject(parent),
    m_emulation(emulation),
    m_regExp(regExp),
    m_forwards(forwards),
    m_startColumn(startColumn),
    m_startLine(startLine)
{
}

HistorySearch::~HistorySearch()
{
}

void HistorySearch::search()
{
    bool found = false;

    if (! m_regExp.isEmpty()) {
        if (m_forwards) {
            found = search(m_startColumn, m_startLine, -1, m_emulation->lineCount()) || search(0, 0, m_startColumn, m_startLine);
        } else {
            found = search(0, 0, m_startColumn, m_startLine) || search(m_startColumn, m_startLine, -1, m_emulation->lineCount());
        }

        if (found) {
            emit matchFound(m_foundStartColumn, m_foundStartLine, m_foundEndColumn, m_foundEndLine, m_loseChinese, m_matchChinese);
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

    // We read process history from (and including) startLine to (and including) endLine in
    // blocks of at most 10K lines so that we do not use unhealthy amounts of memory
    int blockSize;
    while ((blockSize = qMin(10000, linesToRead - linesRead)) > 0) {

        QString string;
        QTextStream searchStream(&string);
        PlainTextDecoder decoder;
        decoder.begin(&searchStream);
        decoder.setRecordLinePositions(true);

        // Calculate lines to read and read them
        int blockStartLine = m_forwards ? startLine + linesRead : endLine - linesRead - blockSize + 1;
        int chunkEndLine = blockStartLine + blockSize - 1;
        m_emulation->writeToStream(&decoder, blockStartLine, chunkEndLine);

        // We search between startColumn in the first line of the string and endColumn in the last
        // line of the string. First we calculate the position (in the string) of endColumn in the
        // last line of the string
        int endPosition;

        // The String that Emulator.writeToStream produces has a newline at the end, and so ends with an
        // empty line - we ignore that.
        int numberOfLinesInString = decoder.linePositions().size() - 1;
        if (numberOfLinesInString > 0 && endColumn > -1) {
            endPosition = decoder.linePositions().at(numberOfLinesInString - 1) + endColumn;
        } else {
            endPosition = string.size();
        }

        // So now we can log for m_regExp in the string between startColumn and endPosition
        int matchStart;
        if (m_forwards) {
            matchStart = string.indexOf(m_regExp, startColumn);
            if (matchStart >= endPosition) {
                matchStart = -1;
            }
        } else {
            matchStart = string.lastIndexOf(m_regExp, endPosition - 1);
            if (matchStart < startColumn) {
                matchStart = -1;
            }
        }

        if (matchStart > -1) {
            int matchEnd = matchStart + m_regExp.matchedLength() - 1;
            qDebug() << "Found in string from" << matchStart << "to" << matchEnd;

            // Translate startPos and endPos to startColum, startLine, endColumn and endLine in history.
            int startLineNumberInString = findLineNumberInString(decoder.linePositions(), matchStart);
            m_foundStartColumn = matchStart - decoder.linePositions().at(startLineNumberInString);
            m_foundStartLine = startLineNumberInString + startLine + linesRead;

            int endLineNumberInString = findLineNumberInString(decoder.linePositions(), matchEnd);
            m_foundEndColumn = matchEnd - decoder.linePositions().at(endLineNumberInString);
            m_foundEndLine = endLineNumberInString + startLine + linesRead;

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
                QString txt = m_regExp.pattern();
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
