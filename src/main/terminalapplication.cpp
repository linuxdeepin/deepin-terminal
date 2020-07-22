#include "terminalapplication.h"
#include "mainwindow.h"
#include "service.h"

// qt
#include <QDebug>

TerminalApplication::TerminalApplication(int &argc, char *argv[]) : DApplication(argc, argv)
{

}

TerminalApplication::~TerminalApplication()
{
    // app结束时，释放Service
    if (nullptr != Service::instance()) {
        delete Service::instance();
    }
}
void TerminalApplication::handleQuitAction()
{
    qDebug() << "handleQuitAction";
    activeWindow()->close();
}
// 该部分代码，大都 用于调试，待整理
bool TerminalApplication::notify(QObject *object, QEvent *event)
{
    bool s = event->spontaneous();
    QString n = object->metaObject()->className();
    if ((event->type() == QEvent::KeyPress || event->type() == QEvent::Shortcut)
            /*&& QString(object->metaObject()->className()) == "MainWindow"*/) {

        QKeyEvent *keyevent = static_cast<QKeyEvent *>(event);
        QString keyString;
        if (!keyevent->spontaneous() && n == "QWidgetWindow") {
            // return  true;
        }
        if (event->type() == QEvent::Shortcut) {
            QShortcutEvent *sevent = static_cast<QShortcutEvent *>(event);
            keyString = QKeySequence(sevent->key()).toString(QKeySequence::PortableText);
            // qDebug()<<"Shortcut"<<sevent->key();
        }

        int uKey = keyevent->key();
        Qt::Key key = static_cast<Qt::Key>(uKey);
        if (key == Qt::Key_unknown) {
            // nothing {unknown key}
        }

        if (key == Qt::Key_Control || key == Qt::Key_Shift || key == Qt::Key_Alt) {
            return false;
        }

        //获取修饰键(Ctrl,Alt,Shift)的状态
        Qt::KeyboardModifiers modifiers = keyevent->modifiers();
        //判断某个修饰键是否被按下化
        if (modifiers & Qt::ShiftModifier)
            uKey += Qt::SHIFT;
        if (modifiers & Qt::ControlModifier)
            uKey += Qt::CTRL;
        if (modifiers & Qt::AltModifier)
            uKey += Qt::ALT;

        QString keyString2 = QKeySequence(uKey).toString(QKeySequence::PortableText);
        qDebug() << keyevent->type() << keyString2 << keyString << object << keyevent->spontaneous() << n<<keyevent->key()
                 <<keyevent->nativeScanCode()<<keyevent->nativeVirtualKey()<<keyevent->nativeModifiers();


        if ((keyevent->modifiers() == Qt::AltModifier ) && keyevent->key() == Qt::Key_M)
        {
            qDebug()<<"Alt+M has triggerd";
            QPoint pos= QPoint(qApp->inputMethod()->cursorRectangle().x(), qApp->inputMethod()->cursorRectangle().y());
            QMouseEvent event1(QEvent::MouseButtonPress, pos, Qt::RightButton, Qt::RightButton, Qt::NoModifier);
            QCoreApplication::sendEvent(object, &event1);
            return true;
        }

        return QApplication::notify(object, event);
    }
    if (event->type() == QEvent::FocusIn) {
       // qDebug() <<"FocusIn:"<<object;
    }
    // qDebug() <<event->type()<< s<<n;
    return QApplication::notify(object, event);
}
