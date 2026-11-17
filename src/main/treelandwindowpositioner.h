// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TREELANDWINDOWPOSITIONER_H
#define TREELANDWINDOWPOSITIONER_H

#include <QObject>
#include <QPointer>

#include <cstdint>

class QPoint;

class QScreen;
class QWidget;

struct wl_display;
struct wl_event_queue;
struct wl_registry;
struct treeland_dde_shell_manager_v2;
struct treeland_dde_shell_surface_v2;

// Pins a toplevel window to the top-left corner of an output through the
// treeland_dde_shell_unstable_v2 protocol.
//
// Wayland clients cannot choose their own toplevel position: QWidget::move()
// is a no-op there and the compositor places the window by itself. Treeland
// therefore exposes a private protocol that accepts a fixed position hint for
// privileged shell clients. The hint only affects the initial placement of a
// surface, so it is sent again whenever Qt recreates the wl_surface (on every
// hide/show cycle).
//
// Outside a Treeland session, or on builds without protocol support, every
// method is a no-op.
class TreelandWindowPositioner : public QObject
{
public:
    explicit TreelandWindowPositioner(QWidget *window, QObject *parent = nullptr);
    ~TreelandWindowPositioner() override;

    static bool isSupported();

    // Resolves the screen the window should be pinned to: prefers the screen
    // under cursorPos and falls back to the widget's current screen.
    static QScreen *screenForPosition(QWidget *window, const QPoint &cursorPos);

    // The output whose top-left corner anchors the position. Must be set
    // before applyPosition() and before the surface is mapped.
    void setTargetScreen(QScreen *screen);

    // Hooks the window's Wayland surface and pins it. Safe to call on every
    // show; the hint is re-sent when the surface is recreated.
    void applyPosition();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    static void onRegistryGlobal(void *data,
                                 wl_registry *registry,
                                 uint32_t name,
                                 const char *interface,
                                 uint32_t version);
    void ensureManager();
    void attachNativeWindow();
    void createShellSurface();
    void destroyShellSurface();

    QWidget *m_window = nullptr;
    QPointer<QScreen> m_screen;
    wl_display *m_display = nullptr;
    wl_event_queue *m_eventQueue = nullptr;
    wl_registry *m_registry = nullptr;
    treeland_dde_shell_manager_v2 *m_manager = nullptr;
    treeland_dde_shell_surface_v2 *m_shellSurface = nullptr;
    bool m_managerQueried = false;
    bool m_eventFilterInstalled = false;
    bool m_nativeWindowAttached = false;
};

#endif // TREELANDWINDOWPOSITIONER_H
