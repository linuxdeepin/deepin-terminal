/*
 * Copyright © 2015 Christian Persch
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
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib.h>

#include "vtetypes.h"

#include <type_traits>

static_assert(sizeof(vte::grid::coords) == 2 * sizeof(long), "vte::grid::coords size wrong");

static_assert(sizeof(vte::grid::span) == 4 * sizeof(long), "vte::grid::span size wrong");

static_assert(std::is_pod<vte::view::coords>::value, "vte::view::coords not POD");
static_assert(sizeof(vte::view::coords) == 2 * sizeof(vte::view::coord_t), "vte::view::coords size wrong");
