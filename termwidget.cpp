#include "termwidget.h"

#include <QVBoxLayout>
#include <QApplication>
#include <QDesktopServices>
#include <QDebug>
#include <QMenu>

#include <DDesktopServices>
#include <dinputdialog.h>

DWIDGET_USE_NAMESPACE

TermWidget::TermWidget(QWidget *parent)
    : QTermWidget(0, parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);

    setHistorySize(5000);

    // set shell program
    QString shell { getenv("SHELL") };
    setShellProgram(shell.isEmpty() ? "/bin/bash" : shell);
    setTerminalOpacity(0.75);
    setScrollBarPosition(QTermWidget::ScrollBarRight);

    // config
    qDebug() << availableColorSchemes();
    setColorScheme("Linux");

#ifndef OLD_QTERMWIDGET_VERSION
// Fsck old qtermial and deepin stable
    setBlinkingCursor(true);
#endif

    connect(this, &QTermWidget::urlActivated, this, [](const QUrl & url, bool fromContextMenu){
        if (QApplication::keyboardModifiers() & Qt::ControlModifier || fromContextMenu) {
            QDesktopServices::openUrl(url);
        }
    });

    connect(this, &QWidget::customContextMenuRequested, this, &TermWidget::customContextMenuCall);

    startShellProgram();
}

void TermWidget::customContextMenuCall(const QPoint &pos)
{
    QMenu menu;

    QList<QAction*> termActions = filterActions(pos);
    for (QAction *& action : termActions) {
        menu.addAction(action);
    }

    if (!menu.isEmpty()) {
        menu.addSeparator();
    }

    // add other actions here.
    if (!selectedText().isEmpty()) {
        menu.addAction(QIcon::fromTheme("edit-copy"), tr("Copy &Selection"), this, [this] {
            copyClipboard();
        });
    }

    menu.addAction(QIcon::fromTheme("edit-paste"), tr("&Paste"), this, [this] {
        pasteClipboard();
    });

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
    if (isFullScreen) {
        menu.addAction(QIcon::fromTheme("view-restore-symbolic"), tr("Exit Full&screen"), this, [this] {
            window()->setWindowState(windowState() & ~Qt::WindowFullScreen);
        });
    } else {
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
        bool ok;
        QString text = DInputDialog::getText(window(), tr("Rename Tab"), tr("Tab name:"),
                                             QLineEdit::Normal, QString(), &ok);
        if (ok) {
            emit termRequestRenameTab(text);
        }
    });

    // display the menu.
    menu.exec(mapToGlobal(pos));
}

TermWidgetWrapper::TermWidgetWrapper(QWidget *parent)
    : QWidget(parent)
{
    initUI();

    connect(m_term, &QTermWidget::titleChanged, this, [this] { emit termTitleChanged(m_term->title()); });
    connect(m_term, &QTermWidget::finished, this, &TermWidgetWrapper::termClosed);
    // proxy signal:
    connect(m_term, &QTermWidget::termGetFocus, this, &TermWidgetWrapper::termGetFocus);
    connect(m_term, &TermWidget::termRequestSplit, this, &TermWidgetWrapper::termRequestSplit);
    connect(m_term, &TermWidget::termRequestRenameTab, this, &TermWidgetWrapper::termRequestRenameTab);
}

bool TermWidgetWrapper::isTitleChanged() const
{
    return m_term->isTitleChanged();
}

QString TermWidgetWrapper::title() const
{
    return m_term->title();
}

void TermWidgetWrapper::initUI()
{
    m_term = new TermWidget(this);
    setFocusProxy(m_term);

    m_layout = new QVBoxLayout;
    setLayout(m_layout);

    m_layout->addWidget(m_term);
    m_layout->setContentsMargins(0, 0, 0, 0);
}
