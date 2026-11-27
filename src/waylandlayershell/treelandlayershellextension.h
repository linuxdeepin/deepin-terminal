// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <qwayland-treeland-layer-shell-extension-unstable-v1.h>

#include <QtWaylandClient/QWaylandClientExtension>

#include <QObject>

namespace TerminalLayerShell {

/**
 * @brief treeland_layer_shell_extension_manager_v1 的客户端封装。
 *
 * 这是 treeland 的扩展协议：客户端把一次拖拽交给合成器执行，合成器通过
 * layer-surface configure 事件驱动 surface 尺寸，避免客户端逐帧 set_size
 * 造成的 round-trip 延迟。
 */
class TreeLandLayerShellExtensionManager
    : public QWaylandClientExtensionTemplate<TreeLandLayerShellExtensionManager>
    , public QtWayland::treeland_layer_shell_extension_manager_v1
{
    Q_OBJECT
public:
    TreeLandLayerShellExtensionManager();

    struct ::treeland_layer_shell_extension_object_v1 *getExtensionObject(struct ::wl_surface *surface);
};

/**
 * @brief 绑定到单个 layer surface 的扩展对象。
 */
class TreeLandLayerShellExtensionObject
    : public QWaylandClientExtensionTemplate<TreeLandLayerShellExtensionObject>
    , public QtWayland::treeland_layer_shell_extension_object_v1
{
    Q_OBJECT
public:
    explicit TreeLandLayerShellExtensionObject(struct ::treeland_layer_shell_extension_object_v1 *object);
    ~TreeLandLayerShellExtensionObject() override;

    void beginResize(struct ::wl_seat *seat,
                     uint32_t serial,
                     uint32_t edges,
                     int32_t minWidth,
                     int32_t minHeight,
                     int32_t maxWidth,
                     int32_t maxHeight);

Q_SIGNALS:
    void resizingChanged(bool resizing);
    void resizeRejected(uint32_t error);

protected:
    void treeland_layer_shell_extension_object_v1_resizing(uint32_t resizing) override;
    void treeland_layer_shell_extension_object_v1_resize_rejected(uint32_t error) override;
};

} // namespace TerminalLayerShell
