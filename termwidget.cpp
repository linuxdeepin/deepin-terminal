#include "termwidget.h"
#include "define.h"
#include "settings.h"
#include "termproperties.h"

#include <QApplication>
#include <QDebug>
#include <QDesktopServices>
#include <QMenu>
#include <QVBoxLayout>

#include <DDesktopServices>
#include <dinputdialog.h>

DWIDGET_USE_NAMESPACE

TermWidget::TermWidget(TermProperties properties, QWidget *parent) : QTermWidget(0, parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);

    setHistorySize(5000);

    // set shell program
    QString shell{ getenv("SHELL") };
    setShellProgram(shell.isEmpty() ? "/bin/bash" : shell);
    setTerminalOpacity(Settings::instance()->opacity());
    setScrollBarPosition(QTermWidget::ScrollBarRight);
    setKeyboardCursorShape(static_cast< QTermWidget::KeyboardCursorShape >(Settings::instance()->cursorShape()));
    if (properties.contains(WorkingDir))
    {
        setWorkingDirectory(properties[WorkingDir].toString());
    }
    if (properties.contains(Execute))
    {
        QString args = properties[Execute].toString();
        qDebug() << args;
        QStringList argList = args.split(QRegExp(QStringLiteral("\\s+")), QString::SkipEmptyParts);
        if (argList.count() > 0)
        {
            setShellProgram(argList.at(0));
            argList.removeAt(0);
            if (argList.count())
            {
                setArgs(argList);
            }
        }
    }

    // config
    setColorScheme(Settings::instance()->colorScheme());

#if !(QTERMWIDGET_VERSION <= QT_VERSION_CHECK(0, 7, 1))
    setBlinkingCursor(Settings::instance()->cursorBlink());
#endif  // !(QTERMWIDGET_VERSION <= QT_VERSION_CHECK(0, 7, 1))

    connect(this, &QTermWidget::urlActivated, this, [](const QUrl &url, bool fromContextMenu) {
        if (QApplication::keyboardModifiers() & Qt::ControlModifier || fromContextMenu)
        {
            QDesktopServices::openUrl(url);
        }
    });

    connect(this, &QWidget::customContextMenuRequested, this, &TermWidget::customContextMenuCall);

    startShellProgram();
}

void TermWidget::customContextMenuCall(const QPoint &pos)
{
    QMenu menu;

    QList< QAction * > termActions = filterActions(pos);
    for (QAction *&action : termActions)
    {
        menu.addAction(action);
    }

    if (!menu.isEmpty())
    {
        menu.addSeparator();
    }

    // add other actions here.
    if (!selectedText().isEmpty())
    {
        menu.addAction(QIcon::fromTheme("edit-copy"), tr("Copy &Selection"), this, [this] { copyClipboard(); });
    }

    menu.addAction(QIcon::fromTheme("edit-paste"), tr("&Paste"), this, [this] { pasteClipboard(); });

    menu.addAction(QIcon::fromTheme("document-open-folder"), tr("&Open File Manager"), this, [this] {
        DDesktopServices::showFolder(QUrl::fromLocalFile(workingDirectory()));
    });

    menu.addSeparator();

    menu.addAction(QIcon::fromTheme("view-split-left-right"), tr("Split &Horizontally"), this, [this] {
        emit termRequestSplit(Qt::Horizontal);
    });

    menu.addAction(QIcon::fromTheme("view-split-top-bottom"), tr("Split &Vertically"), this, [this] {
        emit termRequestSplit(Qt::Vertical);
    });

    menu.addSeparator();

    bool isFullScreen = this->window()->windowState().testFlag(Qt::WindowFullScreen);
    if (isFullScreen)
    {
        menu.addAction(QIcon::fromTheme("view-restore-symbolic"), tr("Exit Full&screen"), this, [this] {
            window()->setWindowState(windowState() & ~Qt::WindowFullScreen);
        });
    }
    else
    {
        menu.addAction(QIcon::fromTheme("view-fullscreen-symbolic"), tr("Full&screen"), this, [this] {
            window()->setWindowState(windowState() | Qt::WindowFullScreen);
        });
    }

    menu.addAction(QIcon::fromTheme("edit-find"), tr("&Find"), this, [this] {
        // this is not a DTK look'n'feel search bar, but nah.
        toggleShowSearchBar();
    });

    menu.addSeparator();

    menu.addAction(QIcon::fromTheme("edit-rename"), tr("Rename Tab"), this, [this] {
        // TODO: Tab name as default text?
        bool    ok;
        QString text =
            DInputDialog::getText(nullptr, tr("Rename Tab"), tr("Tab name:"), QLineEdit::Normal, QString(), &ok);
        if (ok)
        {
            emit termRequestRenameTab(text);
        }
    });

    menu.addAction(QIcon::fromTheme("custom-command"), tr("Custom Command"), this, [this] {
        // TODO: Tab name as default text?
        emit termRequestOpenCustomCommand();
    });

    menu.addSeparator();

    menu.addAction(
        QIcon::fromTheme("settings-configure"), tr("Settings"), this, [this] { emit termRequestOpenSettings(); });

    // display the menu.
    menu.exec(mapToGlobal(pos));
}

TermWidgetWrapper::TermWidgetWrapper(TermProperties properties, QWidget *parent)
    : QWidget(parent), m_term(new TermWidget(properties, this))
{
    initUI();

    connect(m_term, &QTermWidget::titleChanged, this, [this] { emit termTitleChanged(m_term->title()); });
    connect(m_term, &QTermWidget::finished, this, &TermWidgetWrapper::termClosed);
    // proxy signal:
    connect(m_term, &QTermWidget::termGetFocus, this, &TermWidgetWrapper::termGetFocus);
    connect(m_term, &TermWidget::termRequestSplit, this, &TermWidgetWrapper::termRequestSplit);
    connect(m_term, &TermWidget::termRequestRenameTab, this, &TermWidgetWrapper::termRequestRenameTab);
    connect(m_term, &TermWidget::termRequestOpenSettings, this, &TermWidgetWrapper::termRequestOpenSettings);
    connect(m_term, &TermWidget::termRequestOpenCustomCommand, this, &TermWidgetWrapper::termRequestOpenCustomCommand);
    // m_term->selectionChanged()
    // connect(m_term, &TermWidget::copyAvailable, this, &TermWidgetWrapper::AutoCopy);
}

bool TermWidgetWrapper::isTitleChanged() const
{
    return m_term->isTitleChanged();
}

QString TermWidgetWrapper::title() const
{
    return m_term->title();
}

QString TermWidgetWrapper::workingDirectory()
{
    return m_term->workingDirectory();
}

void TermWidgetWrapper::sendText(const QString &text)
{
    return m_term->sendText(text);
}

void TermWidgetWrapper::setTerminalOpacity(qreal opacity)
{
    m_term->setTerminalOpacity(opacity);
    /******* Modify by n014361 wangpeili 2020-01-04: 修正实时设置透明度问题************/
    m_term->hide();
    m_term->show();
    /********************* Modify by n014361 wangpeili End ************************/
}

void TermWidgetWrapper::setColorScheme(const QString &name)
{
    m_term->setColorScheme(name);
}
/*******************************************************************************
 1. @函数:   void TermWidgetWrapper::setTerminalFont(const QString &fontName)
 2. @作者:     n014361 王培利
 3. @日期:     2020-01-10
 4. @说明:     设置字体
*******************************************************************************/
void TermWidgetWrapper::setTerminalFont(const QString &fontName)
{
    QFont font = m_term->getTerminalFont();
    // font.setFixedPitch(true);
    font.setFamily(fontName);

    m_term->setTerminalFont(font);
}
/*******************************************************************************
 1. @函数:   void TermWidgetWrapper::setTerminalFontSize(const int fontSize)
 2. @作者:     n014361 王培利
 3. @日期:     2020-01-10
 4. @说明:     设置字体大小
*******************************************************************************/
void TermWidgetWrapper::setTerminalFontSize(const int fontSize)
{
    QFont font = m_term->getTerminalFont();
    font.setFixedPitch(true);
    font.setPointSize(fontSize);
    m_term->setTerminalFont(font);
}
/*******************************************************************************
 1. @函数:   void TermWidgetWrapper::selectAll()
 2. @作者:     n014361 王培利
 3. @日期:     2020-01-10
 4. @说明:   全选（功能实现有问题）
*******************************************************************************/
void TermWidgetWrapper::selectAll()
{
    int iRowCount     = m_term->historyLinesCount();
    int iColumnsCount = m_term->screenColumnsCount();

    m_term->setSelectionStart(1, 1);
    m_term->setSelectionEnd(iRowCount, iColumnsCount);
    m_term->hide();
    m_term->show();
    qDebug() << "selectAll" << iRowCount << iColumnsCount;
    qDebug() << m_term->availableKeyBindings();
}
/*******************************************************************************
 1. @函数:   void TermWidgetWrapper::skipToNextCommand()
 2. @作者:     n014361 王培利
 3. @日期:     2020-01-10
 4. @说明:    跳转到下一个命令（这个功能没找到库的接口，现在是暂时是以虚拟键形式实现）
*******************************************************************************/
void TermWidgetWrapper::skipToNextCommand()
{
    qDebug() << "skipToNextCommand";
}
/*******************************************************************************
 1. @函数:  void TermWidgetWrapper::skipToPreCommand()
 2. @作者:     n014361 王培利
 3. @日期:     2020-01-10
 4. @说明:   跳转到前一个命令（这个功能没找到库的接口，现在是暂时是以虚拟键形式实现）
*******************************************************************************/
void TermWidgetWrapper::skipToPreCommand()
{
    qDebug() << "skipToPreCommand";
}
/*******************************************************************************
 1. @函数:  void TermWidgetWrapper::setCursorShape(int shape)
 2. @作者:     n014361 王培利
 3. @日期:     2020-01-10
 4. @说明:     设置光标形状
*******************************************************************************/
void TermWidgetWrapper::setCursorShape(int shape)
{
    Konsole::Emulation::KeyboardCursorShape cursorShape = Konsole::Emulation::KeyboardCursorShape(shape);
    m_term->setKeyboardCursorShape(cursorShape);
}
/*******************************************************************************
 1. @函数:   void TermWidgetWrapper::setCursorBlinking(bool enable)
 2. @作者:     n014361 王培利
 3. @日期:     2020-01-10
 4. @说明:   设置光标是否闪烁(有BUG，设置完成后，失去焦点的窗口会一直闪烁）
*******************************************************************************/
void TermWidgetWrapper::setCursorBlinking(bool enable)
{
    m_term->setBlinkingCursor(enable);
    // m_term->unsetCursor();
    // m_term->setCursor();
    emit m_term->termLostFocus();
    // qDebug() << m_term->hasFocus();
    // m_term->update();
    // m_term->show();
    // m_term->clearFocus();
    // emit m_term->termLostFocus();
}

void TermWidgetWrapper::zoomIn()
{
    m_term->zoomIn();
    // m_term->test();
}

void TermWidgetWrapper::zoomOut()
{
    m_term->zoomOut();
}

void TermWidgetWrapper::copyClipboard()
{
    m_term->copyClipboard();
}

void TermWidgetWrapper::pasteClipboard()
{
    m_term->pasteClipboard();
}

/*******************************************************************************
 1. @函数:   void TermWidgetWrapper::pasteSelection()
 2. @作者:     n014361 王培利
 3. @日期:     2020-01-10
 4. @说明:     粘贴选择内容
*******************************************************************************/
void TermWidgetWrapper::pasteSelection()
{
    m_term->pasteSelection();
}

void TermWidgetWrapper::initUI()
{
    setFocusProxy(m_term);

    m_layout = new QVBoxLayout;
    setLayout(m_layout);

    m_layout->addWidget(m_term);
    m_layout->setContentsMargins(0, 0, 0, 0);
}
