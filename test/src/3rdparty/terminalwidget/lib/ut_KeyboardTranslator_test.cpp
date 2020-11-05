#include "ut_KeyboardTranslator_test.h"

#define private public
#include "KeyboardTranslator.h"
#undef private

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QFileInfo>
#include <QDebug>

using namespace Konsole;

UT_KeyboardTranslator_Test::UT_KeyboardTranslator_Test()
{
}

void UT_KeyboardTranslator_Test::SetUp()
{
}

void UT_KeyboardTranslator_Test::TearDown()
{
}

typedef struct {
    QByteArray text;
    QByteArray result;
    bool wildcards;
    Qt::KeyboardModifiers modifiers;
} KeyBoardTranslatorData;

#ifdef UT_KEYBOARDTRANSLATOR_TEST

TEST_F(UT_KeyboardTranslator_Test, KeyboardTranslatorTest)
{
    QList<QByteArray> entry;
    entry << QByteArray("E*")
          << QByteArray("E*")
          << QByteArray("E*")
          << QByteArray("E*");

    QList<QByteArray> resultList;
    resultList << QByteArray("E1") << QByteArray("E2") << QByteArray("E3") << QByteArray("E5");

    QList<Qt::KeyboardModifiers> keyboardModifier;
    keyboardModifier << Qt::NoModifier
                     << Qt::ShiftModifier
                     << Qt::AltModifier
                     << Qt::ControlModifier;

    for(int i=0; i<entry.size(); i++)
    {
        KeyboardTranslator::Entry keyEntry;
        keyEntry.setText(entry.at(i));
        keyEntry.setModifiers(keyboardModifier.at(i));
        qDebug() << i << " : "<< keyEntry.text(true, keyboardModifier.at(i)) << endl;
        EXPECT_EQ(keyEntry.text(true, keyboardModifier.at(i)), resultList.at(i));
    }

    entry.clear();
    entry << QByteArray("\033[24;*~")
          << QByteArray("\033[24;*~")
          << QByteArray("\033[24;*~")
          << QByteArray("\033[24;*~");

    resultList.clear();
    resultList << QByteArray("\033[24;4~") << QByteArray("\033[24;6~") << QByteArray("\033[24;7~") << QByteArray("\033[24;8~");

    keyboardModifier.clear();
    keyboardModifier << (Qt::ShiftModifier | Qt::AltModifier)
                     << (Qt::ShiftModifier | Qt::ControlModifier)
                     << (Qt::ControlModifier | Qt::AltModifier)
                     << (Qt::ShiftModifier | Qt::AltModifier | Qt::ControlModifier);

    for(int i=0; i<entry.size(); i++)
    {
        KeyboardTranslator::Entry keyEntry;
        keyEntry.setText(entry.at(i));
        keyEntry.setModifiers(keyboardModifier.at(i));
        qDebug() << i << " : "<< keyEntry.text(true, keyboardModifier.at(i)) << endl;
        EXPECT_EQ(keyEntry.text(true, keyboardModifier.at(i)), resultList.at(i));
    }
}

TEST_F(UT_KeyboardTranslator_Test, loadTranslatorTest)
{
    KeyboardTranslator *translator = KeyboardTranslatorManager::instance()->loadTranslator(QLatin1String("default"));

    if (!translator) {
        QBuffer textBuffer;
        static const QByteArray defaultTranslatorText;
        textBuffer.setData(defaultTranslatorText);
        textBuffer.open(QIODevice::ReadOnly);
        translator = KeyboardTranslatorManager::instance()->loadTranslator(&textBuffer, QLatin1String("fallback"));
    }
}

TEST_F(UT_KeyboardTranslator_Test, findTranslatorTest)
{
    KeyboardTranslatorManager::instance()->findTranslator("default");
}

TEST_F(UT_KeyboardTranslator_Test, allTranslatorsTest)
{
    QStringList translators = KeyboardTranslatorManager::instance()->allTranslators();
}

TEST_F(UT_KeyboardTranslator_Test, findAddWriteEntryTest)
{
    KeyboardTranslator* translator = new KeyboardTranslator(QLatin1String("default"));

    KeyboardTranslator::Entry entry = translator->findEntry(
                                          Qt::Key_Delete,
                                          Qt::NoModifier,
                                          KeyboardTranslator::NoState);

    KeyboardTranslator::Entry newEntry;
    KeyboardTranslator::States flags = KeyboardTranslator::NoState;
    KeyboardTranslator::States flagMask = KeyboardTranslator::NoState;
    Qt::KeyboardModifiers modifiers = Qt::NoModifier;
    Qt::KeyboardModifiers modifierMask = Qt::NoModifier;
    KeyboardTranslator::Command command = KeyboardTranslator::NoCommand;
    newEntry.setKeyCode(Qt::Key_Delete);
    newEntry.setState(flags);
    newEntry.setStateMask(flagMask);
    newEntry.setModifiers(modifiers);
    newEntry.setModifierMask(modifierMask);
    newEntry.setText(QByteArray("delete", QString("delete").length()));
    newEntry.setCommand(command);
    translator->addEntry(entry);

    QDir dir("/usr/share/terminalwidget5/kb-layouts");
    QStringList filters;
    filters << QLatin1String("*.keytab");

    QStringList list = dir.entryList(filters);
    QString path = list.first();
    EXPECT_EQ(!path.isEmpty(), true);

    QFile destination(path);
    bool isOpenSuccess = destination.open(QIODevice::WriteOnly | QIODevice::Text);
    EXPECT_EQ(isOpenSuccess, true);

    KeyboardTranslatorWriter writer(&destination);

    QListIterator<KeyboardTranslator::Entry> iter(translator->entries());
    while( iter.hasNext() )
    {
        writer.writeEntry(iter.next());
    }

    delete translator;
}

TEST_F(UT_KeyboardTranslator_Test, KeyboardTranslatorReaderTest)
{
    QDir dir("/usr/share/terminalwidget5/kb-layouts");
    QStringList filters;
    filters << QLatin1String("*.keytab");

    QStringList list = dir.entryList(filters);
    QString path = list.first();

    QFile source(path);
    source.open(QIODevice::ReadOnly);

    QFileInfo keyTabFileInfo(path);

    KeyboardTranslator* translator = new KeyboardTranslator(keyTabFileInfo.baseName());

    KeyboardTranslatorReader reader(&source);
    while(reader.hasNextEntry())
    {
        translator->addEntry(reader.nextEntry());
    }

    source.close();

    delete translator;
}

#endif
