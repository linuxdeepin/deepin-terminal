// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "treelandwindowpositioner.h"

#include <QEvent>
#include <QGuiApplication>
#include <QtGui/qevent.h>
#include <QCursor>
#include <QScreen>
#include <QWidget>
#include <QWindow>

#ifdef DDE_SHELL_POSITION_SUPPORT

#include <qpa/qplatformnativeinterface.h>
#include <qpa/qplatformwindow_p.h>

#include <wayland-client.h>

#include "treeland-dde-shell-unstable-v2-client-protocol.h"

namespace {

QWindow *windowOf(QWidget *widget)
{
    return widget ? widget->windowHandle() : nullptr;
}

QNativeInterface::Private::QWaylandWindow *waylandWindowOf(QWidget *widget)
{
    QWindow *window = windowOf(widget);
    if (!window || !window->handle())
        return nullptr;
    return window->nativeInterface<QNativeInterface::Private::QWaylandWindow>();
}

} // namespace

TreelandWindowPositioner::TreelandWindowPositioner(QWidget *window, QObject *parent)
    : QObject(parent)
    , m_window(window)
{
}

TreelandWindowPositioner::~TreelandWindowPositioner()
{
    // Do not send protocol requests here: the wl_surface may already be gone,
    // in which case the compositor has destroyed the shell surface as well.
    if (m_shellSurface) {
        wl_proxy_destroy(reinterpret_cast<wl_proxy *>(m_shellSurface));
        m_shellSurface = nullptr;
    }
    if (m_manager) {
        wl_proxy_destroy(reinterpret_cast<wl_proxy *>(m_manager));
        m_manager = nullptr;
    }
    if (m_registry) {
        wl_registry_destroy(m_registry);
        m_registry = nullptr;
    }
    if (m_eventQueue) {
        wl_event_queue_destroy(m_eventQueue);
        m_eventQueue = nullptr;
    }
}

bool TreelandWindowPositioner::isSupported()
{
    return QGuiApplication::platformName().startsWith(QLatin1String("wayland"));
}

QScreen *TreelandWindowPositioner::screenForPosition(QWidget *window, const QPoint &cursorPos)
{
    if (QScreen *screen = QGuiApplication::screenAt(cursorPos))
        return screen;
    return window ? window->screen() : nullptr;
}

void TreelandWindowPositioner::setTargetScreen(QScreen *screen)
{
    m_screen = screen;
}

void TreelandWindowPositioner::applyPosition()
{
    if (!isSupported() || !m_window)
        return;

    QWindow *window = windowOf(m_window);
    if (!window)
        return;

    if (!m_eventFilterInstalled) {
        window->installEventFilter(this);
        m_eventFilterInstalled = true;
    }

    if (!m_screen)
        m_screen = screenForPosition(m_window, QCursor::pos());

    ensureManager();
    attachNativeWindow();

    auto *nativeWindow = waylandWindowOf(m_window);
    if (m_manager && nativeWindow && nativeWindow->surface())
        createShellSurface();
}

bool TreelandWindowPositioner::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::PlatformSurface) {
        auto *surfaceEvent = static_cast<QPlatformSurfaceEvent *>(event);
        if (surfaceEvent->surfaceEventType() == QPlatformSurfaceEvent::SurfaceCreated) {
            attachNativeWindow();
            if (m_manager)
                createShellSurface();
        }
    }

    return QObject::eventFilter(watched, event);
}

void TreelandWindowPositioner::onRegistryGlobal(void *data,
                                                wl_registry *registry,
                                                uint32_t name,
                                                const char *interface,
                                                uint32_t version)
{
    auto *self = static_cast<TreelandWindowPositioner *>(data);
    if (qstrcmp(interface, treeland_dde_shell_manager_v2_interface.name) != 0)
        return;

    self->m_manager = static_cast<treeland_dde_shell_manager_v2 *>(
        wl_registry_bind(registry,
                         name,
                         &treeland_dde_shell_manager_v2_interface,
                         qMin<uint32_t>(version, 1)));
}

void TreelandWindowPositioner::ensureManager()
{
    if (m_managerQueried)
        return;

    auto *platformNative = QGuiApplication::platformNativeInterface();
    if (!platformNative)
        return;

    m_display = static_cast<wl_display *>(
        platformNative->nativeResourceForWindow("display", windowOf(m_window)));
    if (!m_display)
        return;

    m_managerQueried = true;

    m_eventQueue = wl_display_create_queue(m_display);
    m_registry = wl_display_get_registry(m_display);
    if (!m_eventQueue || !m_registry)
        return;

    wl_proxy_set_queue(reinterpret_cast<wl_proxy *>(m_registry), m_eventQueue);

    static const wl_registry_listener listener = {
        &TreelandWindowPositioner::onRegistryGlobal,
        nullptr,
    };
    wl_registry_add_listener(m_registry, &listener, this);
    wl_display_roundtrip_queue(m_display, m_eventQueue);
}

void TreelandWindowPositioner::attachNativeWindow()
{
    if (m_nativeWindowAttached)
        return;

    auto *window = waylandWindowOf(m_window);
    if (!window)
        return;

    connect(window,
            &QNativeInterface::Private::QWaylandWindow::surfaceCreated,
            this,
            [this] { createShellSurface(); },
            Qt::DirectConnection);
    connect(window,
            &QNativeInterface::Private::QWaylandWindow::surfaceRoleDestroyed,
            this,
            [this] { destroyShellSurface(); },
            Qt::DirectConnection);

    m_nativeWindowAttached = true;
}

void TreelandWindowPositioner::createShellSurface()
{
    if (!m_manager || !isSupported())
        return;

    auto *window = waylandWindowOf(m_window);
    wl_surface *surface = window ? window->surface() : nullptr;
    if (!surface)
        return;

    destroyShellSurface();

    m_shellSurface = treeland_dde_shell_manager_v2_get_shell_surface(m_manager, surface);
    if (!m_shellSurface)
        return;

    // Keep the window in the regular toplevel layer; only its position is
    // being controlled here.
    treeland_dde_shell_surface_v2_set_role(m_shellSurface,
                                           TREELAND_DDE_SHELL_SURFACE_V2_ROLE_NORMAL);

    wl_output *output = nullptr;
    if (m_screen) {
        if (auto *platformNative = QGuiApplication::platformNativeInterface())
            output = static_cast<wl_output *>(
                platformNative->nativeResourceForScreen("output", m_screen));
    }

    treeland_dde_shell_surface_v2_set_position_hint(m_shellSurface, output, 0, 0);
    wl_display_flush(m_display);
}

void TreelandWindowPositioner::destroyShellSurface()
{
    if (!m_shellSurface)
        return;

    treeland_dde_shell_surface_v2_destroy(m_shellSurface);
    m_shellSurface = nullptr;

    if (m_display)
        wl_display_flush(m_display);
}

#else // DDE_SHELL_POSITION_SUPPORT

TreelandWindowPositioner::TreelandWindowPositioner(QWidget *window, QObject *parent)
    : QObject(parent)
    , m_window(window)
{
}

TreelandWindowPositioner::~TreelandWindowPositioner() = default;

bool TreelandWindowPositioner::isSupported()
{
    return false;
}

QScreen *TreelandWindowPositioner::screenForPosition(QWidget *window, const QPoint &cursorPos)
{
    if (QScreen *screen = QGuiApplication::screenAt(cursorPos))
        return screen;
    return window ? window->screen() : nullptr;
}

void TreelandWindowPositioner::setTargetScreen(QScreen *screen)
{
    Q_UNUSED(screen)
}

void TreelandWindowPositioner::applyPosition()
{
}

bool TreelandWindowPositioner::eventFilter(QObject *watched, QEvent *event)
{
    return QObject::eventFilter(watched, event);
}

void TreelandWindowPositioner::onRegistryGlobal(void *,
                                                wl_registry *,
                                                uint32_t,
                                                const char *,
                                                uint32_t)
{
}

void TreelandWindowPositioner::ensureManager()
{
}

void TreelandWindowPositioner::attachNativeWindow()
{
}

void TreelandWindowPositioner::createShellSurface()
{
}

void TreelandWindowPositioner::destroyShellSurface()
{
}

#endif // DDE_SHELL_POSITION_SUPPORT
