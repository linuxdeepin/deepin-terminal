#include "encodelistmodel.h"

#include "qtermwidget/lib/qtermwidget.h"

#include <DLog>

#include <QTextCodec>
#include <QRegExp>

EncodeListModel::EncodeListModel(QObject *parent) : QAbstractListModel(parent)
{
    initEncodeData();
}

int EncodeListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    //
    return m_encodeData.count();
}

QVariant EncodeListModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(role);
    const int row = index.row();
    return m_encodeData[row];
}

void EncodeListModel::initEncodeData()
{
    QList<QByteArray> all = QTextCodec::availableCodecs();
    // m_encodeData = QTextCodec::availableCodecs().toSet().toList();
    QList<QByteArray> showEncodeList;
    // 这是ubuntu18.04支持的编码格式，按国家排列的
//    showEncodeList << "GB18030" << "GB2312" << "GBK"
//                   << "BIG5" << "BIG5-HKSCS" << "EUC-TW"
//                   << "EUC-JP" << "ISO-2022-JP" << "SHIFT_JIS"
//                   << "EUC-KR" << "ISO-2022-KR" << "UHC"
//                   << "IBM864" << "ISO-8859-6" << "MAC_ARABIC" << "WINDOWS-1256"
//                   << "ARMSCII-8"
//                   << "ISO-8859-13" << "ISO-8859-4" << "WINDOWS-1257"
//                   << "ISO-8859-14"
//                   << "IBM-852" << "ISO-8859-2" << "MAC_CE" << "WINDOWS-1250"
//                   << "MAC_CROATIAN"
//                   << "IBM855" << "ISO-8859-5" << "ISO-IR-111" << "ISO-IR-111" << "KOI8-R" << "MAC-CYRILLIC" << "WINDOWS-1251"
//                   << "CP866"
//                   << "KOI8-U" << "MAC_UKRAINIAN"
//                   << "GEORGIAN-PS"
//                   << "ISO-8859-7" << "MAC_GREEK" << "WINDOWS-1253"
//                   << "MAC_GUJARATI"
//                   << "MAC_GURMUKHI"
//                   << "IBM862" << "ISO-8859-8-I" << "MAC_HEBREW" << "WINDOWS-1255"
//                   << "ISO-8859-8"
//                   << "MAC_DEVANAGARI"
//                   << "MAC_ICELANDIC"
//                   << "ISO-8859-10"
//                   << "MAC_FARSI"
//                   << "ISO-8859-16" << "MAC_ROMANIAN"
//                   << "ISO-8859-3"
//                   << "TIS-620"
//                   << "IBM857" << "ISO-8859-9" << "MAC_TURKISH" << "WINDOWS-1254"
//                   << "TCVN" << "VISCII" << "WINDOWS-1258"
//                   << "IBM850" << "ISO-8859-1" << "ISO-8859-15" << "MAC_ROMAN" << "WINDOWS-1252";
    showEncodeList << "UTF-8" << "GB18030" << "GB2312" << "GBK" /*简体中文*/
                   << "BIG5" << "BIG5-HKSCS" //<< "EUC-TW"      /*繁体中文*/
                   << "EUC-JP" << "ISO-2022-JP" << "SHIFT_JIS"  /*日语*/
                   << "EUC-KR" << "ISO-2022-KR" //<< "UHC"      /*韩语*/
                   << "IBM864" << "ISO-8859-6" << "ARABIC" << "WINDOWS-1256"   /*阿拉伯语*/
                   //<< "ARMSCII-8"    /*美国语*/
                   << "ISO-8859-13" << "ISO-8859-4" << "WINDOWS-1257"  /*波罗的海各国语*/
                   << "ISO-8859-14"    /*凯尔特语*/
                   << "IBM-852" << "ISO-8859-2" << "x-mac-CE" << "WINDOWS-1250" /*中欧*/
                   //<< "x-mac-CROATIAN"  /*克罗地亚*/
                   << "IBM855" << "ISO-8859-5"  << "KOI8-R" << "MAC-CYRILLIC" << "WINDOWS-1251" //<< "ISO-IR-111" /*西里尔语*/
                   << "CP866" /*西里尔语或俄语*/
                   << "KOI8-U" << "x-MacUkraine" /*西里尔语或乌克兰语*/
                   //<< "GEORGIAN-PS"
                   << "ISO-8859-7" << "x-mac-GREEK" << "WINDOWS-1253"  /*希腊语*/
                   //<< "x-mac-GUJARATI"
                   //<< "x-mac-GURMUKHI"
                   << "IBM862" << "ISO-8859-8-I" << "WINDOWS-1255"//<< "x-mac-HEBREW"  /*希伯来语*/
                   << "ISO-8859-8" /*希伯来语*/
                   //<< "x-mac-DEVANAGARI"
                   //<< "x-mac-ICELANDIC" /*冰岛语*/
                   << "ISO-8859-10"     /*北欧语*/
                   //<< "x-mac-FARSI"     /*波斯语*/
                   //<< "x-mac-ROMANIAN" //<< "ISO-8859-16" /*罗马尼亚语*/
                   << "ISO-8859-3"      /*西欧语*/
                   << "TIS-620"         /*泰语*/
                   << "IBM857" << "ISO-8859-9" << "x-mac-TURKISH" << "WINDOWS-1254" /*土耳其语*/
                   << "WINDOWS-1258" //<< "TCVN" << "VISCII"  /*越南语*/
                   << "IBM850" << "ISO-8859-1" << "ISO-8859-15" << "x-ROMAN8" << "WINDOWS-1252"; /*西方国家*/


    // meld提供的编码格式,按名称排列的
//    showEncodeList<<"UTF-8"
//                 <<"ISO-8859-1"
//                 <<"ISO-8859-2"
//                 <<"ISO-8859-3"
//                 <<"ISO-8859-4"
//                 <<"ISO-8859-5"
//                 <<"ISO-8859-6"
//                 <<"ISO-8859-7"
//                 <<"ISO-8859-8"
//                 <<"ISO-8859-9"
//                 <<"ISO-8859-10"
//                 <<"ISO-8859-13"
//                 <<"ISO-8859-14"
//                 <<"ISO-8859-15"
//                 //<<"ISO-8859-16"
//                 <<"UTF-7"
//                 <<"UTF-16"
//                 <<"UTF-16BE"
//                 <<"UTF-16LE"
//                 <<"UTF-32"
//                 <<"UCS-2"
//                 <<"UCS-4"
//                 //<<"ARMSCII-8"
//                 <<"BIG5"
//                 <<"BIG5-HKSCS"
//                 <<"CP866"
//                 <<"EUC-JP"
//                 //<<"EUC-JP-MS"
//                 <<"CP932"
//                 <<"EUC-KR"
//                 //<<"EUC-TW"
//                 <<"GB18030"
//                 <<"GB2312"
//                 <<"GBK"
//                 //<<"GEORGIAN-ACADEMY"
//                 <<"IBM850"
//                 <<"IBM852"
//                 <<"IBM855"
//                 <<"IBM857"
//                 <<"IBM862"
//                 <<"IBM864"
//                 <<"ISO-2022-JP"
//                 <<"ISO-2022-KR"
//                 //<<"ISO-IR-111"
//                 //<<"JOHAB"
//                 <<"KOI8-R"
//                 <<"KOI8-U"
//                 <<"SHIFT_JIS"
//                 //<<"TCVN"
//                 <<"TIS-620"
//                 //<<"UHC"
//                 //<<"VISCII"
//                 <<"WINDOWS-1250"
//                 <<"WINDOWS-1251"
//                 <<"WINDOWS-1252"
//                 <<"WINDOWS-1253"
//                 <<"WINDOWS-1254"
//                 <<"WINDOWS-1255"
//                 <<"WINDOWS-1256"
//                 <<"WINDOWS-1257"
//                 <<"WINDOWS-1258";


    // 自定义的名称，系统里不一定大小写完全一样，再同步一下。
    for (QByteArray name : showEncodeList) {
        QString strname1 = name;
        bool bFind = false;
        QByteArray encodename;
        for (QByteArray name2 : all) {
            QString strname2 = name2;
            if (strname1.compare(strname2, Qt::CaseInsensitive) == 0) {
                bFind = true;
                encodename = name2;
                break;
            }
        }
        if (!bFind) {
            qDebug() << "encode name :" << name << "not find!";
        } else {
            m_encodeData << encodename;
        }
    }
    //qDebug()<<"all encode name :"<<all;
    //m_encodeData = showEncodeList;
    qDebug() << "QTextCodec::availableCodecs" << m_encodeData.count();
}
