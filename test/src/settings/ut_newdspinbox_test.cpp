#include "ut_newdspinbox_test.h"

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QDebug>
#include <QFile>

#include <DSettings>

DCORE_USE_NAMESPACE

UT_NewDSpinBox_Test::UT_NewDSpinBox_Test()
{
}

void UT_NewDSpinBox_Test::SetUp()
{
    m_spinBox = new NewDspinBox(nullptr);
    m_spinBox->show();
}

void UT_NewDSpinBox_Test::TearDown()
{
    delete m_spinBox;
}

#ifdef UT_NEWDSPINBOX_TEST

TEST_F(UT_NewDSpinBox_Test, WheelEvent_Increase)
{
#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
    int value = 20;
    m_spinBox->setValue(value);
#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
    QWheelEvent *e = new QWheelEvent(QPointF(63, 29), 120, Qt::NoButton, Qt::NoModifier);
    m_spinBox->wheelEvent(e);

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
}

TEST_F(UT_NewDSpinBox_Test, WheelEvent_Reduce)
{
#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
    int value = 20;
    m_spinBox->setValue(value);
#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
    QWheelEvent *e = new QWheelEvent(QPointF(63, 29), -120, Qt::NoButton, Qt::NoModifier);
    m_spinBox->wheelEvent(e);

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
}

TEST_F(UT_NewDSpinBox_Test, eventFilter_Key_Up)
{
    int value = 20;
    m_spinBox->setValue(value);
    QKeyEvent *key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier, QString(""));
    m_spinBox->eventFilter(m_spinBox->lineEdit(), key_event);
}

TEST_F(UT_NewDSpinBox_Test, eventFilter_Key_Down)
{
    int value = 20;
    m_spinBox->setValue(value);
    QKeyEvent *key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier, QString(""));
    m_spinBox->eventFilter(m_spinBox->lineEdit(), key_event);
}

#endif
