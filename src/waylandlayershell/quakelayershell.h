// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QPointer>

class QScreen;
class QWidget;

namespace TerminalLayerShell {

class LayerShellWindow;

/**
 * @brief Quake-terminal flavour of a layer-shell surface.
 *
 * The surface is anchored to the top edge (Top|Left|Right, exclusive zone 0,
 * top layer) so Treeland can slide it in from the top on map and back out on
 * unmap.  Hiding therefore unmaps the wl_surface with a null buffer instead of
 * destroying it, which is what lets the compositor play its close animation.
 */
class QuakeLayerShell : public QObject
{
    Q_OBJECT

public:
    explicit QuakeLayerShell(QWidget *window);
    ~QuakeLayerShell() override;

    /**
     * Creates the native window and installs the layer-shell shell integration.
     * Returns false when the session is not Wayland or the compositor has no
     * zwlr_layer_shell_v1; callers then keep the old xdg_toplevel behaviour.
     */
    bool attach();

    bool isActive() const;
    bool isMapped() const;

    /**
     * Remembers the screen the quake window should appear on.  It is applied
     * when the surface is mapped next, never while it is hidden.
     */
    void setScreen(QScreen *screen);
    QScreen *screen() const;

    /**
     * Sets the height of the quake window (the compositor owns the width
     * because the surface is anchored to the left and right edges).
     */
    void updateHeight(int height);

    /**
     * Maps the surface.  A stale, already unmapped platform surface is
     * recreated so Qt starts with fresh frame-callback state; the first commit
     * with a buffer triggers the compositor's open animation.
     */
    void map();

    /**
     * Hides the surface by destroying the layer surface (like dde-grand-search
     * does), so Treeland keeps the SurfaceWrapper alive for its close animation
     * and slides the last frame back out through the top edge.
     */
    void unmap();

Q_SIGNALS:
    /** A compositor-driven interactive resize ended (height changed). */
    void interactiveResizeFinished();
    /** The compositor rejected a compositor-driven resize request. */
    void interactiveResizeRejected();

private:
    void onSurfaceDismissed();
    void remapAfterDismiss();

    QWidget *m_window = nullptr;
    LayerShellWindow *m_layerShellWindow = nullptr;
    QPointer<QScreen> m_screen;
    bool m_mapped = false;
    bool m_dismissed = false;
};

} // namespace TerminalLayerShell
