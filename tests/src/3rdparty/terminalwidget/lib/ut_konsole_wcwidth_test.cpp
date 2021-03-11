/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:     wangliang <wangliang@uniontech.com>
 *
 * Maintainer: wangliang <wangliang@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ut_konsole_wcwidth_test.h"

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QDebug>

using namespace Konsole;

UT_KonsoleWcwidth_Test::UT_KonsoleWcwidth_Test()
{
}

void UT_KonsoleWcwidth_Test::SetUp()
{
}

void UT_KonsoleWcwidth_Test::TearDown()
{
}

#ifdef UT_QTERMWIDGET_TEST

class PainterWidget :  public QWidget
{
    Q_OBJECT
public:
    explicit PainterWidget(QWidget* parent = nullptr) : QWidget(parent)
    {
    }
protected:
    void paintEvent(QPaintEvent *event)
    {
        Q_UNUSED(event);
        QPainter painter(this);

        int fontWidth = 15;
        int fontHeight = 15;

        QStringList drawStringList;
        QString str = QString("abcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()");
        QString str2 = QString("╌╍╎╏┄┅┆┇┈┉┊┋╚╝╔ ╗╠ ╩ ╣ ╦╬╨╞ ╥ ╡║ ╫ ═ ╪");
        QString str3 = QString("╄  ╃ ╆ ╅┗  ┛ ┏ ┓└  ┘ ┌ ┐┖  ┙ ┍ ┒┚  ┕ ┎ ┑╘  ╜ ╓ ╕ ╛  ╙ ╒ ╖");
        QString str4 = QString("▖▗▘▙▚▛▜▝▞▞▟");
        drawStringList << str << str2 << str3 << str4;
        QRect cellRect = {0, 0, fontWidth, fontHeight};
        bool bold = true;
        for (int strIndex = 0; strIndex < drawStringList.size(); strIndex++) {
            QString drawStr = drawStringList.at(strIndex);
            for (int i = 0 ; i < drawStr.length(); i++) {
                static const ushort FirstBoxDrawingCharacterCodePoint = 0x2500;
                QChar chr = drawStr[i];
                const uchar code = chr.unicode() - FirstBoxDrawingCharacterCodePoint;

                int x = cellRect.x();
                int y = cellRect.y();
                int w = cellRect.width();
                int h = cellRect.height();

                LineBlockCharacters::drawBasicLineCharacter(painter, x, y, w, h, code, bold);
                LineBlockCharacters::drawDashedLineCharacter(painter, x, y, w, h, code, bold);
                LineBlockCharacters::drawRoundedCornerLineCharacter(painter, x, y, w, h, code, bold);
                LineBlockCharacters::drawDiagonalLineCharacter(painter, x, y, w, h, code, bold);
                LineBlockCharacters::drawBlockCharacter(painter, x, y, w, h, code, bold);

                int codeArray[] = {0x80, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95};
                for(int codeIndex=0; codeIndex<5; codeIndex++) {
                    LineBlockCharacters::drawBlockCharacter(painter, x, y, w, h, codeArray[codeIndex], bold);
                }
            }
        }
    }
};

/*******************************************************************************
 1. @函数:    konsole_wcwidth.cpp文件中
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    characterWidth单元测试
*******************************************************************************/
TEST_F(UT_KonsoleWcwidth_Test, characterWidth)
{
    for (uint i = 0; i < 255; i++) {
        int width = characterWidth(i);
        EXPECT_GE(width, -1);
    }
}

TEST_F(UT_KonsoleWcwidth_Test, drawCharacter)
{
    PainterWidget painterWidget;
    painterWidget.resize(800, 600);
    painterWidget.show();

    QPaintEvent *event = new QPaintEvent(painterWidget.rect());
    painterWidget.paintEvent(event);
}

#include "ut_konsole_wcwidth_test.moc"

#endif
