// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "layershellwindow.h"

#include <qwayland-wlr-layer-shell-unstable-v1.h>

#include <QtWaylandClient/private/qwaylandshellsurface_p.h>
#include <QtWaylandClient/private/qwaylandwindow_p.h>

namespace TerminalLayerShell {

class TreeLandLayerShellExtensionObject;

class QWaylandLayerShellSurface
    : public QtWaylandClient::QWaylandShellSurface
    , public QtWayland::zwlr_layer_surface_v1
{
    Q_OBJECT
public:
    QWaylandLayerShellSurface(QtWayland::zwlr_layer_shell_v1 *shell,
                              QtWaylandClient::QWaylandWindow *window);
    ~QWaylandLayerShellSurface() override;

    bool isExposed() const override { return m_configured; }

    void applyConfigure() override;
    void setWindowGeometry(const QRect &geometry) override;
    void attachPopup(QWaylandShellSurface *popup) override;

    bool resize(QtWaylandClient::QWaylandInputDevice *seat, Qt::Edges edges) override;
    bool requestActivate() override;
    bool requestActivateOnShow() override { return true; }

private:
    void zwlr_layer_surface_v1_configure(uint32_t serial, uint32_t width, uint32_t height) override;
    void zwlr_layer_surface_v1_closed() override;

    QtWaylandClient::QWaylandWindow *waylandWindow();
    QWindow *windowHandle();
    QtWaylandClient::QWaylandScreen *waylandScreen();
    wl_output *currentOutput();
    void calcAndSetRequestSize(QSize requestSize);
    bool anchorsSizeConflict() const;
    void trySetAnchorsAndSize();
    void applyLayer();
    void applyExclusiveZone();
    void applyMargins();
    void applyKeyboardInteractivity();
    void applyInputRegion();
    void commitWindowState();
    void flushCommit();
    void scheduleCommit();
    void recreateWindow();
    void flushRecreate();
    void scheduleRecreate();

    LayerShellWindow *m_layerShellWindow = nullptr;
    TreeLandLayerShellExtensionObject *m_extensionObject = nullptr;
    QSize m_pendingSize;
    QSize m_requestSize;
    wl_output *m_output = nullptr;
    bool m_configured = false;
    bool m_commitScheduled = false;
    bool m_recreateScheduled = false;
};

} // namespace TerminalLayerShell
