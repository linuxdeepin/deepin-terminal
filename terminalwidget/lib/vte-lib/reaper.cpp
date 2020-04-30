/*
 * Copyright (C) 2002 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include "reaper.h"

struct _VteReaper {
    GObject parent_instance;
};

struct _VteReaperClass {
    GObjectClass parent_class;
};
typedef struct _VteReaperClass VteReaperClass;

#define VTE_TYPE_REAPER            (vte_reaper_get_type())
#define VTE_REAPER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), VTE_TYPE_REAPER, VteReaper))
#define VTE_REAPER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  VTE_TYPE_REAPER, VteReaperClass))
#define VTE_IS_REAPER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), VTE_TYPE_REAPER))
#define VTE_IS_REAPER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  VTE_TYPE_REAPER))
#define VTE_REAPER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  VTE_TYPE_REAPER, VteReaperClass))

static GType vte_reaper_get_type(void);

G_DEFINE_TYPE(VteReaper, vte_reaper, G_TYPE_OBJECT)

static VteReaper *singleton_reaper = nullptr;

static void
vte_reaper_child_watch_cb(GPid pid,
                          int status,
                          gpointer data)
{
    g_signal_emit_by_name(data, "child-exited", pid, status);
    g_spawn_close_pid(pid);
}

/*
 * vte_reaper_add_child:
 * @pid: the ID of a child process which will be monitored
 *
 * Ensures that child-exited signals will be emitted when @pid exits.
 */
void
vte_reaper_add_child(GPid pid)
{
    g_child_watch_add_full(G_PRIORITY_LOW,
                           pid,
                           vte_reaper_child_watch_cb,
                           vte_reaper_ref(),
                           (GDestroyNotify)g_object_unref);
}

static void
vte_reaper_init(VteReaper *reaper)
{
}

static GObject *
vte_reaper_constructor(GType                  type,
                       guint                  n_construct_properties,
                       GObjectConstructParam *construct_properties)
{
    if (singleton_reaper) {
        return (GObject *)g_object_ref(singleton_reaper);
    } else {
        GObject *obj;
        obj = G_OBJECT_CLASS(vte_reaper_parent_class)->constructor(type, n_construct_properties, construct_properties);
        singleton_reaper = VTE_REAPER(obj);
        return obj;
    }
}


static void
vte_reaper_finalize(GObject *reaper)
{
    G_OBJECT_CLASS(vte_reaper_parent_class)->finalize(reaper);
    singleton_reaper = nullptr;
}

static void
vte_reaper_class_init(VteReaperClass *klass)
{
    GObjectClass *gobject_class;

    /*
     * VteReaper::child-exited:
     * @vtereaper: the object which received the signal
     * @arg1: the process ID of the exited child
     * @arg2: the status of the exited child, as returned by waitpid()
     *
     * Emitted when the #VteReaper object detects that a child of the
     * current process has exited.
     */
    g_signal_new(g_intern_static_string("child-exited"),
                 G_OBJECT_CLASS_TYPE(klass),
                 G_SIGNAL_RUN_LAST,
                 0,
                 nullptr,
                 nullptr,
                 g_cclosure_marshal_generic,
                 G_TYPE_NONE,
                 2,
                 G_TYPE_INT, G_TYPE_INT);

    gobject_class = G_OBJECT_CLASS(klass);
    gobject_class->constructor = vte_reaper_constructor;
    gobject_class->finalize = vte_reaper_finalize;
}

/*
 * vte_reaper_ref:
 *
 * Finds the address of the global #VteReaper object, creating the object if
 * necessary.
 *
 * Returns: a reference to the global #VteReaper object, which
 *  must be unreffed when done with it
 */
VteReaper *
vte_reaper_ref(void)
{
    return (VteReaper *)g_object_new(VTE_TYPE_REAPER, nullptr);
}
