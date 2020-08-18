#include "ut_termwidget_test.h"

#define private public
#include "service.h"
#include "termwidget.h"
#include "termwidgetpage.h"
#include "qtermwidget.h"
#include "TerminalDisplay.h"
#undef private

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QSignalSpy>
#include <QDebug>

UT_TermWidget_Test::UT_TermWidget_Test()
{
}

void UT_TermWidget_Test::SetUp()
{
    if (!Service::instance()->property("isServiceInit").toBool())
    {
        Service::instance()->init();
        Service::instance()->setProperty("isServiceInit", true);
    }

    m_normalTermProperty[QuakeMode] = false;
    m_normalTermProperty[SingleFlag] = true;
    m_normalWindow = new NormalWindow(m_normalTermProperty, nullptr);
}

void UT_TermWidget_Test::TearDown()
{
    delete m_normalWindow;
}

#ifdef UT_TERMWIDGET_TEST
TEST_F(UT_TermWidget_Test, TermWidgetTest)
{
    m_normalWindow->resize(800, 600);
    m_normalWindow->show();
    EXPECT_EQ(m_normalWindow->isVisible(), true);

    TermWidgetPage *currTermPage = m_normalWindow->currentPage();
    EXPECT_EQ(currTermPage->isVisible(), true);

    TermWidget *termWidget = currTermPage->m_currentTerm;

    bool isInRemoteServer = termWidget->isInRemoteServer();
    EXPECT_EQ(isInRemoteServer, false);

    TermWidgetPage *termPage = termWidget->parentPage();
    EXPECT_NE(termPage, nullptr);
    EXPECT_EQ(QString(termPage->metaObject()->className()), QString("TermWidgetPage"));

    //这两个函数未实现
    termWidget->skipToPreCommand();
    termWidget->skipToNextCommand();

    //设置透明度
    for(qreal opacity=0.01; opacity <= 1.0; opacity += 0.01)
    {
        termWidget->setTermOpacity(opacity);
#ifdef ENABLE_UI_TEST
        QTest::qWait(10);
#endif
    }

    QStringList fontFamilyList;
    fontFamilyList << "Courier 10 Pitch" << "DejaVu Sans Mono" << "Liberation Mono"
              << "Noto Mono" << "Noto Sans Mono" << "Noto Sans Mono CJK JP"
              << "Noto Sans Mono CJK KR" << "Noto Sans Mono CJK SC"
              << "Noto Sans Mono CJK TC";
    for(int i=0; i<fontFamilyList.size(); i++)
    {
        QString fontFamily = fontFamilyList.at(i);
        termWidget->setTermFont(fontFamily);
        QFont currFont = termWidget->getTerminalFont();
        EXPECT_EQ(currFont.family(), fontFamily);

#ifdef ENABLE_UI_TEST
        QTest::qWait(200);
#endif
    }

    QString lastFontFamily = fontFamilyList.last();
    QFont currFont = termWidget->getTerminalFont();
    EXPECT_EQ(currFont.family(), lastFontFamily);

    //字体大小大于20时界面提示符显示会有异常
    //设置字体大小时会不停刷日志：Using a variable-width font in the terminal.  This may cause performance degradation and display/alignment errors.
    for(int fontSize=5; fontSize<=50; fontSize++)
    {
        termWidget->setTermFontSize(fontSize);
        QFont currFont = termWidget->getTerminalFont();
        EXPECT_EQ(currFont.pointSize(), fontSize);
#ifdef ENABLE_UI_TEST
        QTest::qWait(100);
#endif
    }

#ifdef ENABLE_UI_TEST
    QTest::qWait(1000);
#endif
}

TEST_F(UT_TermWidget_Test, TermWidgetTest2)
{
    m_normalWindow->resize(800, 600);
    m_normalWindow->show();
    EXPECT_EQ(m_normalWindow->isVisible(), true);

    TermWidgetPage *currTermPage = m_normalWindow->currentPage();
    EXPECT_EQ(currTermPage->isVisible(), true);

    TermWidget *termWidget = currTermPage->m_currentTerm;

    QList<QByteArray> encodeList;
    encodeList << "UTF-8" << "GB18030" << "GB2312" << "GBK" /*简体中文*/
               << "BIG5" << "BIG5-HKSCS" //<< "EUC-TW"      /*繁体中文*/
               << "EUC-JP"  << "SHIFT_JIS"  //<< "ISO-2022-JP"/*日语*/
               << "EUC-KR" //<< "ISO-2022-KR" //<< "UHC"      /*韩语*/
               << "IBM864" << "ISO-8859-6" << "ARABIC" << "WINDOWS-1256"   /*阿拉伯语*/
               << "ISO-8859-13" << "ISO-8859-4" << "WINDOWS-1257"  /*波罗的海各国语*/
               << "ISO-8859-14"    /*凯尔特语*/
               << "IBM-852" << "ISO-8859-2" << "x-mac-CE" << "WINDOWS-1250" /*中欧*/
               << "IBM855" << "ISO-8859-5"  << "KOI8-R" << "MAC-CYRILLIC" << "WINDOWS-1251" //<< "ISO-IR-111" /*西里尔语*/
               << "CP866" /*西里尔语或俄语*/
               << "KOI8-U" << "x-MacUkraine" /*西里尔语或乌克兰语*/
               << "ISO-8859-7" << "x-mac-GREEK" << "WINDOWS-1253"  /*希腊语*/
               << "IBM862" << "ISO-8859-8-I" << "WINDOWS-1255"//<< "x-mac-HEBREW"  /*希伯来语*/
               << "ISO-8859-8" /*希伯来语*/
               << "ISO-8859-10"     /*北欧语*/
               << "ISO-8859-3"      /*西欧语*/
               << "TIS-620"         /*泰语*/
               << "IBM857" << "ISO-8859-9" << "x-mac-TURKISH" << "WINDOWS-1254" /*土耳其语*/
               << "WINDOWS-1258" //<< "TCVN" << "VISCII"  /*越南语*/
               << "IBM850" << "ISO-8859-1" << "ISO-8859-15" << "x-ROMAN8" << "WINDOWS-1252"; /*西方国家*/

    for(int i=0; i<encodeList.size(); i++)
    {
        QString encode = QString(encodeList.at(i));
        termWidget->setEncode(encode);
        EXPECT_EQ(termWidget->encode(), encode);
        qDebug() << termWidget->encode();

        termWidget->setRemoteEncode(encode);
        EXPECT_EQ(termWidget->RemoteEncode(), encode);
        qDebug() << termWidget->RemoteEncode();
    }
}

#endif
