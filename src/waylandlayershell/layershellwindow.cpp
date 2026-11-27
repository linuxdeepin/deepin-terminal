// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "layershellwindow.h"
#include "qwaylandlayershellintegration_p.h"

#include <QLoggingCategory>

#include <QtWaylandClient/private/qwaylandwindow_p.h>

Q_LOGGING_CATEGORY(lcLayerShellWindow, "deepin.terminal.layershell.window")

namespace TerminalLayerShell {

class LayerShellWindowPrivate
{
public:
    explicit LayerShellWindowPrivate(QWindow *window)
        : parentWindow(window)
    {
    }

    QWindow *parentWindow;
    QString scope;
    LayerShellWindow::Anchors anchors = LayerShellWindow::AnchorNone;
    int32_t exclusionZone = 0;
    LayerShellWindow::KeyboardInteractivity keyboardInteractivity =
        LayerShellWindow::KeyboardInteractivityNone;
    LayerShellWindow::Layer layer = LayerShellWindow::LayerTop;
    int leftMargin = 0;
    int rightMargin = 0;
    int topMargin = 0;
    int bottomMargin = 0;
    LayerShellWindow::ScreenConfiguration screenConfiguration =
        LayerShellWindow::ScreenFromQWindow;
    bool closeOnDismissed = false;
    bool active = false;
    QRegion inputRegion;
};

LayerShellWindow::LayerShellWindow(QWindow *window)
    : QObject(window)
    , d(new LayerShellWindowPrivate(window))
{
    window->create();

    auto *waylandWindow = dynamic_cast<QtWaylandClient::QWaylandWindow *>(window->handle());
    if (!waylandWindow) {
        qCWarning(lcLayerShellWindow) << "not a Wayland window, no zwlr_layer_surface will be created";
        return;
    }

    static QWaylandLayerShellIntegration *shellIntegration = nullptr;
    if (!shellIntegration) {
        shellIntegration = new QWaylandLayerShellIntegration();
        if (!shellIntegration->initialize(waylandWindow->display())) {
            delete shellIntegration;
            shellIntegration = nullptr;
            qCWarning(lcLayerShellWindow) << "compositor does not support zwlr_layer_shell_v1";
            return;
        }
    }

    waylandWindow->setShellIntegration(shellIntegration);
    d->active = true;
}

LayerShellWindow::~LayerShellWindow() = default;

void LayerShellWindow::setAnchors(Anchors anchors)
{
    if (anchors != d->anchors) {
        d->anchors = anchors;
        Q_EMIT anchorsChanged();
    }
}

LayerShellWindow::Anchors LayerShellWindow::anchors() const
{
    return d->anchors;
}

void LayerShellWindow::setExclusiveZone(int32_t zone)
{
    if (zone != d->exclusionZone) {
        d->exclusionZone = zone;
        Q_EMIT exclusionZoneChanged();
    }
}

int32_t LayerShellWindow::exclusionZone() const
{
    return d->exclusionZone;
}

void LayerShellWindow::setLeftMargin(int marginLeft)
{
    if (marginLeft != d->leftMargin) {
        d->leftMargin = marginLeft;
        Q_EMIT marginsChanged();
    }
}

int LayerShellWindow::leftMargin() const
{
    return d->leftMargin;
}

void LayerShellWindow::setRightMargin(int marginRight)
{
    if (marginRight != d->rightMargin) {
        d->rightMargin = marginRight;
        Q_EMIT marginsChanged();
    }
}

int LayerShellWindow::rightMargin() const
{
    return d->rightMargin;
}

void LayerShellWindow::setTopMargin(int marginTop)
{
    if (marginTop != d->topMargin) {
        d->topMargin = marginTop;
        Q_EMIT marginsChanged();
    }
}

int LayerShellWindow::topMargin() const
{
    return d->topMargin;
}

void LayerShellWindow::setBottomMargin(int marginBottom)
{
    if (marginBottom != d->bottomMargin) {
        d->bottomMargin = marginBottom;
        Q_EMIT marginsChanged();
    }
}

int LayerShellWindow::bottomMargin() const
{
    return d->bottomMargin;
}

void LayerShellWindow::setKeyboardInteractivity(KeyboardInteractivity interactivity)
{
    if (interactivity != d->keyboardInteractivity) {
        d->keyboardInteractivity = interactivity;
        Q_EMIT keyboardInteractivityChanged();
    }
}

LayerShellWindow::KeyboardInteractivity LayerShellWindow::keyboardInteractivity() const
{
    return d->keyboardInteractivity;
}

void LayerShellWindow::setLayer(Layer layer)
{
    if (layer != d->layer) {
        d->layer = layer;
        Q_EMIT layerChanged();
    }
}

LayerShellWindow::Layer LayerShellWindow::layer() const
{
    return d->layer;
}

void LayerShellWindow::setScreenConfiguration(ScreenConfiguration screenConfiguration)
{
    d->screenConfiguration = screenConfiguration;
}

LayerShellWindow::ScreenConfiguration LayerShellWindow::screenConfiguration() const
{
    return d->screenConfiguration;
}

void LayerShellWindow::setScope(const QString &scope)
{
    if (scope != d->scope) {
        d->scope = scope;
        Q_EMIT scopeChanged();
    }
}

QString LayerShellWindow::scope() const
{
    return d->scope;
}

void LayerShellWindow::setInputRegion(const QRegion &region)
{
    if (region != d->inputRegion) {
        d->inputRegion = region;
        Q_EMIT inputRegionChanged();
    }
}

void LayerShellWindow::resetInputRegion()
{
    setInputRegion(QRegion());
}

QRegion LayerShellWindow::inputRegion() const
{
    return d->inputRegion;
}

void LayerShellWindow::setCloseOnDismissed(bool close)
{
    d->closeOnDismissed = close;
}

bool LayerShellWindow::closeOnDismissed() const
{
    return d->closeOnDismissed;
}

bool LayerShellWindow::isActive() const
{
    return d->active;
}

QWindow *LayerShellWindow::window() const
{
    return d->parentWindow;
}

void LayerShellWindow::notifyDismissed()
{
    Q_EMIT surfaceDismissed();
}

void LayerShellWindow::notifyInteractiveResizeFinished()
{
    Q_EMIT interactiveResizeFinished();
}

void LayerShellWindow::notifyInteractiveResizeRejected()
{
    Q_EMIT interactiveResizeRejected();
}

LayerShellWindow *LayerShellWindow::get(QWindow *window)
{
    if (auto *existing = qobject_cast<LayerShellWindow *>(window->findChild<LayerShellWindow *>(QString(), Qt::FindDirectChildrenOnly))) {
        return existing;
    }

    return new LayerShellWindow(window);
}

} // namespace TerminalLayerShell
