// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QRegion>
#include <QWindow>

class QScreen;

namespace TerminalLayerShell {

class LayerShellWindowPrivate;

/**
 * @brief Per-window attachment of the zwlr_layer_shell_v1 role.
 *
 * Adapted from dde-shell (frame/layershell/dlayershellwindow.*).  Unlike a
 * QPA shell integration plugin, this is installed on a single QWindow so an
 * application can mix regular xdg_toplevel windows and layer surfaces.
 */
class LayerShellWindow : public QObject
{
    Q_OBJECT

public:
    ~LayerShellWindow() override;

    enum Anchor {
        AnchorNone = 0,
        AnchorTop = 1,
        AnchorBottom = 2,
        AnchorLeft = 4,
        AnchorRight = 8
    };
    Q_ENUM(Anchor)
    Q_DECLARE_FLAGS(Anchors, Anchor)

    enum Layer {
        LayerBackground = 0,
        LayerBottom = 1,
        LayerTop = 2,
        LayerOverlay = 3,
    };
    Q_ENUM(Layer)

    enum KeyboardInteractivity {
        KeyboardInteractivityNone = 0,
        KeyboardInteractivityExclusive = 1,
        KeyboardInteractivityOnDemand = 2,
    };
    Q_ENUM(KeyboardInteractivity)

    enum ScreenConfiguration {
        ScreenFromQWindow = 0,
        ScreenFromCompositor = 1,
    };
    Q_ENUM(ScreenConfiguration)

    void setAnchors(Anchors anchors);
    Anchors anchors() const;

    void setExclusiveZone(int32_t zone);
    int32_t exclusionZone() const;

    void setLeftMargin(int marginLeft);
    int leftMargin() const;
    void setRightMargin(int marginRight);
    int rightMargin() const;
    void setTopMargin(int marginTop);
    int topMargin() const;
    void setBottomMargin(int marginBottom);
    int bottomMargin() const;

    void setKeyboardInteractivity(KeyboardInteractivity interactivity);
    KeyboardInteractivity keyboardInteractivity() const;

    void setLayer(Layer layer);
    Layer layer() const;

    void setScreenConfiguration(ScreenConfiguration screenConfiguration);
    ScreenConfiguration screenConfiguration() const;

    void setScope(const QString &scope);
    QString scope() const;

    void setInputRegion(const QRegion &region);
    void resetInputRegion();
    QRegion inputRegion() const;

    /**
     * Whether the compositor should close the QWindow when the layer surface
     * is dismissed (for example when its output is removed).
     */
    void setCloseOnDismissed(bool close);
    bool closeOnDismissed() const;

    /**
     * True when the QWindow is a Wayland window and the compositor advertised
     * zwlr_layer_shell_v1.
     */
    bool isActive() const;

    QWindow *window() const;

    /**
     * Called by the surface when the compositor dismissed it.
     */
    void notifyDismissed();

    /**
     * Called by the surface when a compositor-driven resize starts/ends.
     */
    void notifyInteractiveResizeFinished();
    void notifyInteractiveResizeRejected();

    /**
     * Gets the LayerShellWindow for a given QWindow. Ownership is not transferred;
     * the object is parented to the QWindow.
     */
    static LayerShellWindow *get(QWindow *window);

Q_SIGNALS:
    void anchorsChanged();
    void exclusionZoneChanged();
    void marginsChanged();
    void keyboardInteractivityChanged();
    void layerChanged();
    void scopeChanged();
    void inputRegionChanged();
    void surfaceDismissed();
    void interactiveResizeFinished();
    void interactiveResizeRejected();

private:
    explicit LayerShellWindow(QWindow *window);

    QScopedPointer<LayerShellWindowPrivate> d;
};

} // namespace TerminalLayerShell
