// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "qwaylandlayershellsurface_p.h"
#include "treelandlayershellextension.h"

#include <qwayland-wlr-layer-shell-unstable-v1.h>
#include <wayland-wlr-layer-shell-unstable-v1-client-protocol.h>

#include <QLoggingCategory>
#include <QPointer>

#include <QtWaylandClient/private/qwaylandinputdevice_p.h>
#include <QtWaylandClient/private/qwaylandscreen_p.h>
#include <QtWaylandClient/private/qwaylandsurface_p.h>
#include <QtWaylandClient/private/qwaylandwindow_p.h>

Q_LOGGING_CATEGORY(lcLayerShellSurface, "deepin.terminal.layershell.surface")

namespace TerminalLayerShell {

static TreeLandLayerShellExtensionManager *extensionManager()
{
    static QPointer<TreeLandLayerShellExtensionManager> manager;
    if (!manager)
        manager = new TreeLandLayerShellExtensionManager();
    return manager;
}

QWaylandLayerShellSurface::QWaylandLayerShellSurface(QtWayland::zwlr_layer_shell_v1 *shell,
                                                     QtWaylandClient::QWaylandWindow *window)
    : QtWaylandClient::QWaylandShellSurface(window)
    , QtWayland::zwlr_layer_surface_v1()
    , m_layerShellWindow(LayerShellWindow::get(window->window()))
{
    wl_output *output = nullptr;
    if (m_layerShellWindow->screenConfiguration() == LayerShellWindow::ScreenFromQWindow) {
        output = currentOutput();
        connect(window->window(), &QWindow::screenChanged, this, &QWaylandLayerShellSurface::scheduleRecreate);
        if (!output) {
            qCWarning(lcLayerShellSurface) << "failed to get wl_output for the window screen";
        }
    }

    init(shell->get_layer_surface(window->waylandSurface()->object(),
                                  output,
                                  m_layerShellWindow->layer(),
                                  m_layerShellWindow->scope()));
    m_output = output;

    // Bind the treeland interactive-resize extension early so the first drag
    // does not race the registry round-trip.
    (void)extensionManager();

    applyLayer();
    connect(m_layerShellWindow, &LayerShellWindow::layerChanged, this, &QWaylandLayerShellSurface::applyLayer);

    set_anchor(m_layerShellWindow->anchors());
    connect(m_layerShellWindow, &LayerShellWindow::anchorsChanged, this, &QWaylandLayerShellSurface::trySetAnchorsAndSize);

    applyExclusiveZone();
    connect(m_layerShellWindow, &LayerShellWindow::exclusionZoneChanged, this, &QWaylandLayerShellSurface::applyExclusiveZone);

    applyMargins();
    connect(m_layerShellWindow, &LayerShellWindow::marginsChanged, this, &QWaylandLayerShellSurface::applyMargins);

    applyKeyboardInteractivity();
    connect(m_layerShellWindow, &LayerShellWindow::keyboardInteractivityChanged, this, &QWaylandLayerShellSurface::applyKeyboardInteractivity);

    applyInputRegion();
    connect(m_layerShellWindow, &LayerShellWindow::inputRegionChanged,
            this, &QWaylandLayerShellSurface::applyInputRegion);

    calcAndSetRequestSize(window->surfaceSize());

    if (m_requestSize.isValid()) {
        set_size(m_requestSize.width(), m_requestSize.height());
    }
}

QWaylandLayerShellSurface::~QWaylandLayerShellSurface()
{
    // The extension object is tied to the wl_surface and must not outlive it.
    delete m_extensionObject;
    m_extensionObject = nullptr;
    destroy();
}

void QWaylandLayerShellSurface::zwlr_layer_surface_v1_closed()
{
    if (m_layerShellWindow->closeOnDismissed()) {
        window()->window()->close();
    } else {
        m_layerShellWindow->notifyDismissed();
    }
}

bool QWaylandLayerShellSurface::requestActivate()
{
    if (!m_layerShellWindow)
        return false;

    const auto interactivity = m_layerShellWindow->keyboardInteractivity();
    if (interactivity == LayerShellWindow::KeyboardInteractivityNone)
        return false;

    // Treeland grants keyboard focus when a layer surface regains focus
    // capability (mapped && keyboard_interactivity != none).  There is no
    // activation request for layer surfaces, so force a capability transition
    // with two consecutive commits.
    m_layerShellWindow->setKeyboardInteractivity(LayerShellWindow::KeyboardInteractivityNone);
    commitWindowState();
    m_layerShellWindow->setKeyboardInteractivity(interactivity);
    commitWindowState();
    return true;
}

bool QWaylandLayerShellSurface::resize(QtWaylandClient::QWaylandInputDevice *seat, Qt::Edges edges)
{
    // Only the bottom edge is unanchored (Top|Left|Right), so it is the only
    // edge the compositor can drag for the quake window.
    if (!seat || edges != Qt::BottomEdge)
        return false;

    auto *manager = extensionManager();
    if (!manager || !manager->isActive()) {
        qCInfo(lcLayerShellSurface) << "treeland layer shell extension unavailable, falling back to client-side resize";
        return false;
    }

    auto *waylandSurface = window()->waylandSurface();
    if (!waylandSurface)
        return false;

    if (!m_extensionObject) {
        auto *object = manager->getExtensionObject(waylandSurface->object());
        if (!object)
            return false;

        m_extensionObject = new TreeLandLayerShellExtensionObject(object);
        connect(m_extensionObject, &TreeLandLayerShellExtensionObject::resizingChanged,
                this, [this](bool resizing) {
                    if (!resizing && m_layerShellWindow)
                        m_layerShellWindow->notifyInteractiveResizeFinished();
                });
        connect(m_extensionObject, &TreeLandLayerShellExtensionObject::resizeRejected,
                this, [this](uint32_t) {
                    if (m_layerShellWindow)
                        m_layerShellWindow->notifyInteractiveResizeRejected();
                });
    }

    const uint32_t serial = getSerial(seat);
    if (serial == 0) {
        qCWarning(lcLayerShellSurface) << "no pointer serial available for interactive resize";
        return false;
    }

    auto *handle = windowHandle();
    int minHeight = handle ? handle->minimumHeight() : 0;
    int maxHeight = handle ? handle->maximumHeight() : 0;
    // The extension uses 0 for "unbounded".
    if (maxHeight >= 16777215) // QWIDGETSIZE_MAX
        maxHeight = 0;

    m_extensionObject->beginResize(seat->wl_seat(), serial,
                                   TREELAND_LAYER_SHELL_EXTENSION_OBJECT_V1_RESIZE_EDGE_BOTTOM,
                                   0, minHeight, 0, maxHeight);
    qCInfo(lcLayerShellSurface) << "compositor-driven resize requested, min height =" << minHeight
                                << "max height =" << maxHeight;
    return true;
}

QtWaylandClient::QWaylandWindow *QWaylandLayerShellSurface::waylandWindow()
{
    return window();
}

QWindow *QWaylandLayerShellSurface::windowHandle()
{
    auto *currentWindow = waylandWindow();
    return currentWindow ? currentWindow->window() : nullptr;
}

QtWaylandClient::QWaylandScreen *QWaylandLayerShellSurface::waylandScreen()
{
    auto *currentWindowHandle = windowHandle();
    if (!currentWindowHandle || !currentWindowHandle->screen())
        return nullptr;

    return dynamic_cast<QtWaylandClient::QWaylandScreen *>(currentWindowHandle->screen()->handle());
}

wl_output *QWaylandLayerShellSurface::currentOutput()
{
    auto *currentWaylandScreen = waylandScreen();
    return currentWaylandScreen ? currentWaylandScreen->output() : nullptr;
}

void QWaylandLayerShellSurface::calcAndSetRequestSize(QSize requestSize)
{
    const auto anchors = m_layerShellWindow->anchors();
    const bool horizontallyConstrained = anchors.testFlags({LayerShellWindow::AnchorLeft, LayerShellWindow::AnchorRight});
    const bool verticallyConstrained = anchors.testFlags({LayerShellWindow::AnchorTop, LayerShellWindow::AnchorBottom});
    m_requestSize = requestSize;
    if (horizontallyConstrained)
        m_requestSize.setWidth(0);
    if (verticallyConstrained)
        m_requestSize.setHeight(0);
}

bool QWaylandLayerShellSurface::anchorsSizeConflict() const
{
    const auto anchors = m_layerShellWindow->anchors();
    const bool horizontallyConstrained = anchors.testFlags({LayerShellWindow::AnchorLeft, LayerShellWindow::AnchorRight});
    const bool verticallyConstrained = anchors.testFlags({LayerShellWindow::AnchorTop, LayerShellWindow::AnchorBottom});
    return (!horizontallyConstrained && m_requestSize.width() == 0)
        || (!verticallyConstrained && m_requestSize.height() == 0);
}

void QWaylandLayerShellSurface::trySetAnchorsAndSize()
{
    if (!anchorsSizeConflict()) {
        set_anchor(m_layerShellWindow->anchors());
        set_size(m_requestSize.width(), m_requestSize.height());
        scheduleCommit();
    }
}

void QWaylandLayerShellSurface::applyLayer()
{
    set_layer(m_layerShellWindow->layer());
    scheduleCommit();
}

void QWaylandLayerShellSurface::applyExclusiveZone()
{
    set_exclusive_zone(m_layerShellWindow->exclusionZone());
    scheduleCommit();
}

void QWaylandLayerShellSurface::applyMargins()
{
    set_margin(m_layerShellWindow->topMargin(),
               m_layerShellWindow->rightMargin(),
               m_layerShellWindow->bottomMargin(),
               m_layerShellWindow->leftMargin());
    scheduleCommit();
}

void QWaylandLayerShellSurface::applyKeyboardInteractivity()
{
    set_keyboard_interactivity(m_layerShellWindow->keyboardInteractivity());
    scheduleCommit();
}

void QWaylandLayerShellSurface::applyInputRegion()
{
    auto *currentWindowHandle = windowHandle();
    if (!currentWindowHandle)
        return;

    currentWindowHandle->setMask(m_layerShellWindow->inputRegion());
    scheduleCommit();
}

void QWaylandLayerShellSurface::zwlr_layer_surface_v1_configure(uint32_t serial,
                                                                uint32_t width,
                                                                uint32_t height)
{
    ack_configure(serial);
    m_pendingSize = QSize(width, height);

    if (!m_configured) {
        m_configured = true;
        window()->resizeFromApplyConfigure(m_pendingSize);
#if QT_VERSION < QT_VERSION_CHECK(6, 7, 0)
        window()->handleExpose(QRect(QPoint(), m_pendingSize));
#elif QT_VERSION < QT_VERSION_CHECK(6, 9, 0)
        window()->sendRecursiveExposeEvent();
#else
        window()->updateExposure();
#endif
    } else {
        window()->applyConfigureWhenPossible();
    }
}

void QWaylandLayerShellSurface::applyConfigure()
{
    window()->resizeFromApplyConfigure(m_pendingSize);
}

void QWaylandLayerShellSurface::setWindowGeometry(const QRect &geometry)
{
    calcAndSetRequestSize(geometry.size());
    trySetAnchorsAndSize();
}

void QWaylandLayerShellSurface::commitWindowState()
{
    auto *currentWaylandWindow = waylandWindow();
    if (!currentWaylandWindow || !currentWaylandWindow->waylandSurface())
        return;

    currentWaylandWindow->waylandSurface()->commit();
}

void QWaylandLayerShellSurface::flushCommit()
{
    m_commitScheduled = false;
    commitWindowState();
}

void QWaylandLayerShellSurface::scheduleCommit()
{
    if (m_commitScheduled)
        return;

    m_commitScheduled = true;
    QMetaObject::invokeMethod(this, &QWaylandLayerShellSurface::flushCommit, Qt::QueuedConnection);
}

void QWaylandLayerShellSurface::recreateWindow()
{
    auto *currentWaylandWindow = waylandWindow();
    if (!currentWaylandWindow)
        return;

    currentWaylandWindow->reset();
    QMetaObject::invokeMethod(currentWaylandWindow, &QtWaylandClient::QWaylandWindow::reinit, Qt::QueuedConnection);
}

void QWaylandLayerShellSurface::flushRecreate()
{
    m_recreateScheduled = false;
    recreateWindow();
}

void QWaylandLayerShellSurface::scheduleRecreate()
{
    auto *currentWindowHandle = windowHandle();
    if (!currentWindowHandle)
        return;

    const auto output = currentOutput();
    if (!output) {
        qCWarning(lcLayerShellSurface) << "failed to get wl_output for the new screen";
        return;
    }

    if (output == m_output || m_recreateScheduled)
        return;

    m_output = output;
    m_recreateScheduled = true;
    QMetaObject::invokeMethod(this, &QWaylandLayerShellSurface::flushRecreate, Qt::QueuedConnection);
}

void QWaylandLayerShellSurface::attachPopup(QtWaylandClient::QWaylandShellSurface *popup)
{
    std::any anyRole = popup->surfaceRole();

    if (auto role = std::any_cast<::xdg_popup *>(&anyRole)) {
        get_popup(*role);
    } else {
        qCWarning(lcLayerShellSurface) << "Cannot attach popup of unknown type";
    }
}

} // namespace TerminalLayerShell
