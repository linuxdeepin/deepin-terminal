// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <qwayland-wlr-layer-shell-unstable-v1.h>

#include <private/qwaylandshellintegration_p.h>

namespace TerminalLayerShell {

class QWaylandLayerShellIntegration
    : public QtWaylandClient::QWaylandShellIntegrationTemplate<QWaylandLayerShellIntegration>
    , public QtWayland::zwlr_layer_shell_v1
{
public:
    QWaylandLayerShellIntegration();
    ~QWaylandLayerShellIntegration() override;

    QtWaylandClient::QWaylandShellSurface *createShellSurface(
        QtWaylandClient::QWaylandWindow *window) override;
};

} // namespace TerminalLayerShell
