// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "quakelayershell.h"
#include "layershellwindow.h"

#include <QGuiApplication>
#include <QLoggingCategory>
#include <QScreen>
#include <QTimer>
#include <QWidget>
#include <QWindow>

Q_LOGGING_CATEGORY(lcQuakeLayerShell, "deepin.terminal.layershell.quake")

using namespace TerminalLayerShell;

QuakeLayerShell::QuakeLayerShell(QWidget *window)
    : QObject(window)
    , m_window(window)
{
}

QuakeLayerShell::~QuakeLayerShell() = default;

bool QuakeLayerShell::attach()
{
    Q_ASSERT(m_window);

    if (!QGuiApplication::platformName().startsWith(QStringLiteral("wayland"))) {
        qCInfo(lcQuakeLayerShell) << "not running on Wayland, keep the xdg_toplevel behaviour";
        return false;
    }

    // Force the platform window to exist before the shell integration is set.
    m_window->winId();
    auto *windowHandle = m_window->windowHandle();
    if (!windowHandle) {
        qCWarning(lcQuakeLayerShell) << "no QWindow available";
        return false;
    }

    m_layerShellWindow = LayerShellWindow::get(windowHandle);
    if (!m_layerShellWindow->isActive()) {
        delete m_layerShellWindow;
        m_layerShellWindow = nullptr;
        return false;
    }

    m_layerShellWindow->setScope(QStringLiteral("deepin-terminal/quake"));
    // Exactly Top|Left|Right: treeland derives the exclusive edge (and thus the
    // slide animation direction) from this combination, and left+right stretch
    // the surface to the full work-area width.
    m_layerShellWindow->setAnchors(LayerShellWindow::Anchors(LayerShellWindow::AnchorTop
                                                             | LayerShellWindow::AnchorLeft
                                                             | LayerShellWindow::AnchorRight));
    m_layerShellWindow->setExclusiveZone(0);
    m_layerShellWindow->setLayer(LayerShellWindow::LayerTop);
    m_layerShellWindow->setKeyboardInteractivity(LayerShellWindow::KeyboardInteractivityOnDemand);
    m_layerShellWindow->setCloseOnDismissed(false);
    m_layerShellWindow->setScreenConfiguration(LayerShellWindow::ScreenFromQWindow);

    connect(m_layerShellWindow, &LayerShellWindow::surfaceDismissed,
            this, &QuakeLayerShell::onSurfaceDismissed);
    connect(m_layerShellWindow, &LayerShellWindow::interactiveResizeFinished,
            this, &QuakeLayerShell::interactiveResizeFinished);
    connect(m_layerShellWindow, &LayerShellWindow::interactiveResizeRejected,
            this, &QuakeLayerShell::interactiveResizeRejected);

    m_screen = windowHandle->screen();
    qCInfo(lcQuakeLayerShell) << "quake window attached to wlr-layer-shell";
    return true;
}

bool QuakeLayerShell::isActive() const
{
    return m_layerShellWindow && m_layerShellWindow->isActive();
}

bool QuakeLayerShell::isMapped() const
{
    return m_mapped;
}

void QuakeLayerShell::setScreen(QScreen *screen)
{
    if (!screen || screen == m_screen.data())
        return;

    m_screen = screen;
}

QScreen *QuakeLayerShell::screen() const
{
    return m_screen.data();
}

void QuakeLayerShell::updateHeight(int height)
{
    if (!m_window || height <= 0)
        return;

    m_window->resize(m_window->width(), height);
}

void QuakeLayerShell::map()
{
    Q_ASSERT(m_window);

    if (!isActive()) {
        m_window->show();
        m_mapped = true;
        return;
    }

    if (m_mapped) {
        m_window->activateWindow();
        return;
    }

    qCInfo(lcQuakeLayerShell) << "mapping quake layer surface on screen"
                              << (m_screen ? m_screen->name() : QString());

    m_window->setUpdatesEnabled(true);

    auto *windowHandle = m_window->windowHandle();
    if (windowHandle && m_screen && windowHandle->screen() != m_screen.data())
        windowHandle->setScreen(m_screen.data());

    // Drop the unmapped platform surface from the previous show/hide cycle.
    // Qt's frame-callback state cannot be recovered while the surface has no
    // buffer, so recreate it; the compositor only animates the new commit.
    if (m_window->isVisible())
        m_window->hide();

    m_window->show();
    // Attach the first buffer synchronously so the compositor sees the map
    // transition and plays the slide-in animation.
    m_window->repaint();
    m_mapped = true;
}

void QuakeLayerShell::unmap()
{
    Q_ASSERT(m_window);

    if (!m_mapped)
        return;

    if (!isActive()) {
        m_window->hide();
        m_mapped = false;
        return;
    }

    // Hide (and thereby destroy) the layer surface.  Treeland keeps the
    // SurfaceWrapper alive for the close animation and snapshots the last
    // frame, so the window slides back out through the top edge.  Destroying
    // rather than attaching a null buffer matters: an unmap clears the
    // surface's buffer before the compositor takes its snapshot.
    qCInfo(lcQuakeLayerShell) << "hiding quake layer surface (compositor close animation)";
    m_window->setUpdatesEnabled(false);
    m_window->hide();
    m_mapped = false;
}

void QuakeLayerShell::onSurfaceDismissed()
{
    if (m_dismissed)
        return;

    m_dismissed = true;
    QTimer::singleShot(0, this, &QuakeLayerShell::remapAfterDismiss);
}

void QuakeLayerShell::remapAfterDismiss()
{
    const bool wasMapped = m_mapped;
    m_dismissed = false;

    // The compositor took the layer surface away (output removed).  Destroy
    // the platform surface through Qt, then map again on the current screen.
    if (m_window->isVisible())
        m_window->hide();

    m_mapped = false;

    if (wasMapped)
        map();
}
