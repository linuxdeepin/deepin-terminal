#include "terminalapplication.h"
#include "mainwindow.h"
#include "service.h"

// qt
#include <QDebug>

TerminalApplication::TerminalApplication(int &argc, char *argv[]) : DApplication(argc, argv)
{
    loadTranslator();
    setOrganizationName("deepin");
    setOrganizationDomain("deepin.org");
    setApplicationVersion(VERSION);
    setApplicationName("deepin-terminal");
    setApplicationDisplayName(QObject::tr("Terminal"));
    setAttribute(Qt::AA_UseHighDpiPixmaps, true);
    setProductIcon(QIcon::fromTheme("deepin-terminal"));
    QString appDesc = QObject::tr("Terminal is an advanced terminal emulator with workspace"
                                  ", multiple windows, remote management, quake mode and other features.");
    setApplicationDescription(appDesc);

    /***add by ut001121 zhangmeng 20200617 禁用应用程序自动退出 修复BUG33541***/
    setQuitOnLastWindowClosed(false);

#ifdef QT_DEBUG
    QTranslator translator;
    translator.load(QString("deepin-terminal_%1").arg(QLocale::system().name()));
    installTranslator(&translator);
#endif  // QT_DEBUG
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
    // ALT+M = 右键
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyevent = static_cast<QKeyEvent *>(event);
        if ((keyevent->modifiers() == Qt::AltModifier) && keyevent->key() == Qt::Key_M) {
            // 光标中心点
            QPoint pos = QPoint(qApp->inputMethod()->cursorRectangle().x() + qApp->inputMethod()->cursorRectangle().width() / 2,
                             qApp->inputMethod()->cursorRectangle().y() + qApp->inputMethod()->cursorRectangle().height() / 2);

            qDebug() << "Alt+M has triggerd" << pos << qApp->inputMethod();
            // QPoint(0,0) 表示无法获取光标位置
            if (pos != QPoint(0, 0)) {
                QMouseEvent event1(QEvent::MouseButtonPress, pos, Qt::RightButton, Qt::NoButton, Qt::NoModifier);
                QCoreApplication::sendEvent(object, &event1);
            }

            return true;
        }

        return QApplication::notify(object, event);
    }
#if 0
    // 快捷键检测
    bool spont = event->spontaneous();
    //qDebug() <<event->type()<< spont<<classname;
    QString classname = object->metaObject()->className();
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
        qDebug() << keyevent->type() << keyString2 << keyString << object << keyevent->spontaneous() << classname << keyevent->key()
                 << keyevent->nativeScanCode() << keyevent->nativeVirtualKey() << keyevent->nativeModifiers();

    }
#endif


#if 0
    // 焦点检测
    if (event->type() == QEvent::FocusIn) {
        qDebug() << "FocusIn:" << object;
    }
#endif

    return QApplication::notify(object, event);
}
