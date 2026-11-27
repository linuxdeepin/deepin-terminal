// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "treelandlayershellextension.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(lcLayerShellExtension, "deepin.terminal.layershell.extension")

namespace TerminalLayerShell {

TreeLandLayerShellExtensionManager::TreeLandLayerShellExtensionManager()
    : QWaylandClientExtensionTemplate<TreeLandLayerShellExtensionManager>(
          treeland_layer_shell_extension_manager_v1_interface.version)
{
}

struct ::treeland_layer_shell_extension_object_v1 *TreeLandLayerShellExtensionManager::getExtensionObject(
    struct ::wl_surface *surface)
{
    return get_layer_shell_extension_object(surface);
}

TreeLandLayerShellExtensionObject::TreeLandLayerShellExtensionObject(
    struct ::treeland_layer_shell_extension_object_v1 *object)
    : QWaylandClientExtensionTemplate<TreeLandLayerShellExtensionObject>(
          treeland_layer_shell_extension_object_v1_interface.version)
{
    init(object);
}

TreeLandLayerShellExtensionObject::~TreeLandLayerShellExtensionObject()
{
    if (object())
        destroy();
}

void TreeLandLayerShellExtensionObject::beginResize(struct ::wl_seat *seat,
                                                    uint32_t serial,
                                                    uint32_t edges,
                                                    int32_t minWidth,
                                                    int32_t minHeight,
                                                    int32_t maxWidth,
                                                    int32_t maxHeight)
{
    begin_resize(seat, serial, edges, minWidth, minHeight, maxWidth, maxHeight);
}

void TreeLandLayerShellExtensionObject::treeland_layer_shell_extension_object_v1_resizing(uint32_t resizing)
{
    Q_EMIT resizingChanged(resizing != 0);
}

void TreeLandLayerShellExtensionObject::treeland_layer_shell_extension_object_v1_resize_rejected(uint32_t error)
{
    qCWarning(lcLayerShellExtension) << "interactive resize rejected, error =" << error;
    Q_EMIT resizeRejected(error);
}

} // namespace TerminalLayerShell
