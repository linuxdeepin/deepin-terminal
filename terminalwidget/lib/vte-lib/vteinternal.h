/*
 * Copyright (C) 2001-2004 Red Hat, Inc.
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

#pragma once

/* BEGIN sanity checks */

/* Some distributions pass -fexceptions in a way that overrides vte's
 * own -fno-exceptions. This is a hard error; fail the build.
 * See https://gitlab.gnome.org/GNOME/gnome-build-meta/issues/207
 */

/* END sanity checks */

#include <glib.h>

#include "vtedefines.h"
#include "ring.h"
#include <list>
#include <queue>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#ifdef WITH_ICU
#include "icu-converter.hh"
#endif

enum {
    VTE_BIDI_FLAG_IMPLICIT   = 1 << 0,
    VTE_BIDI_FLAG_RTL        = 1 << 1,
    VTE_BIDI_FLAG_AUTO       = 1 << 2,
    VTE_BIDI_FLAG_BOX_MIRROR = 1 << 3,
    VTE_BIDI_FLAG_ALL        = (1 << 4) - 1,
};

namespace vte {

// This is like std::clamp, except that it doesn't throw when
// max_v<min_v, but instead returns min_v in that case.
template<typename T>
constexpr inline T const &
clamp(T const &v,
      T const &min_v,
      T const &max_v)
{
    return std::max(std::min(v, max_v), min_v);
}

enum class SelectionType {
    eCHAR,
    eWORD,
    eLINE,
};

typedef enum _VteCharacterReplacement {
    VTE_CHARACTER_REPLACEMENT_NONE,
    VTE_CHARACTER_REPLACEMENT_LINE_DRAWING
} VteCharacterReplacement;

struct VteScreen {
public:
    VteScreen(gulong max_rows,
              bool has_streams) :
        m_ring{max_rows, has_streams},
        row_data(&m_ring),
        cursor{0, 0}
    {
    }

    vte::base::Ring m_ring; /* buffer contents */
    VteRing *row_data;
    VteVisualPosition cursor;  /* absolute value, from the beginning of the terminal history */
    double scroll_delta{0.0}; /* scroll offset */
    long insert_delta{0}; /* insertion offset */

    /* Stuff saved along with the cursor */
    struct {
        VteVisualPosition cursor;  /* onscreen coordinate, that is, relative to insert_delta */
        uint8_t modes_ecma;
        bool reverse_mode;
        bool origin_mode;
        VteCell defaults;
        VteCell color_defaults;
        VteCharacterReplacement character_replacements[2];
        VteCharacterReplacement *character_replacement;
    } saved;
};

/* For unified handling of PRIMARY and CLIPBOARD selection */
typedef enum {
    VTE_SELECTION_PRIMARY,
    VTE_SELECTION_CLIPBOARD,
    LAST_VTE_SELECTION
} VteSelection;

/* Used in the GtkClipboard API, to distinguish requests for HTML and TEXT
 * contents of a clipboard */
typedef enum {
    VTE_TARGET_TEXT,
    VTE_TARGET_HTML,
    LAST_VTE_TARGET
} VteSelectionTarget;

struct vte_scrolling_region {
    int start, end;
};

namespace util {

class restore_errno
{
public:
    restore_errno() { m_errsv = errno; }
    ~restore_errno() { errno = m_errsv; }
    operator int () const { return m_errsv; }
private:
    int m_errsv;
};

class smart_fd
{
public:
    constexpr smart_fd() noexcept = default;
    explicit constexpr smart_fd(int fd) noexcept : m_fd {fd} { }
    ~smart_fd() noexcept { if (m_fd != -1) { restore_errno errsv; close(m_fd); } }

    inline smart_fd &operator = (int rhs) noexcept { if (m_fd != -1) { restore_errno errsv; close(m_fd); } m_fd = rhs; return *this; }
    inline smart_fd &operator = (smart_fd &rhs) noexcept { if (&rhs != this) { if (m_fd != -1) { restore_errno errsv; close(m_fd); } m_fd = rhs.m_fd; rhs.m_fd = -1; } return *this; }
    inline constexpr operator int () const noexcept { return m_fd; }
//    inline constexpr operator int *() noexcept { assert(m_fd == -1); return &m_fd; }

    int steal() noexcept { auto d = m_fd; m_fd = -1; return d; }

    /* Prevent accidents */
    smart_fd(smart_fd const &) = delete;
    smart_fd(smart_fd &&) = delete;
    smart_fd &operator = (smart_fd const &) = delete;
    smart_fd &operator = (smart_fd &&) = delete;

private:
    int m_fd{-1};
};

} /* namespace util */

}

namespace vte::glib {

class Error
{
public:
    Error() = default;
    ~Error() { reset(); }

    operator GError *()  noexcept { return m_error; }
    operator GError **() noexcept { return &m_error; }

    bool error()   const noexcept { return m_error != nullptr; }
    GQuark domain()  const noexcept { return error() ? m_error->domain : 0; }
    gint code()    const noexcept { return error() ? m_error->code : -1; }
    gchar *message() const noexcept { return error() ? m_error->message : nullptr; }

    void assert_no_error() const noexcept { g_assert_no_error(m_error); }

    bool matches(GQuark domain, int code) const noexcept
    {
        return error() && g_error_matches(m_error, domain, code);
    }

    void reset() noexcept { g_clear_error(&m_error); }

    bool propagate(GError **error) noexcept { g_propagate_error(error, m_error); m_error = nullptr; return false; }

private:
    GError *m_error{nullptr};
};

}

extern GTimer *process_timer;

#define VTE_TEST_FLAG_DECRQCRA (G_GUINT64_CONSTANT(1) << 0)
