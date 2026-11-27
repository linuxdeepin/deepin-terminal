// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "qwaylandlayershellintegration_p.h"
#include "qwaylandlayershellsurface_p.h"

namespace TerminalLayerShell {

QWaylandLayerShellIntegration::QWaylandLayerShellIntegration()
    : QWaylandShellIntegrationTemplate<QWaylandLayerShellIntegration>(4)
{
}

QWaylandLayerShellIntegration::~QWaylandLayerShellIntegration()
{
    if (object() && zwlr_layer_shell_v1_get_version(object()) >= ZWLR_LAYER_SHELL_V1_DESTROY_SINCE_VERSION) {
        zwlr_layer_shell_v1_destroy(object());
    }
}

QtWaylandClient::QWaylandShellSurface *QWaylandLayerShellIntegration::createShellSurface(
    QtWaylandClient::QWaylandWindow *window)
{
    return new QWaylandLayerShellSurface(this, window);
}

} // namespace TerminalLayerShell
