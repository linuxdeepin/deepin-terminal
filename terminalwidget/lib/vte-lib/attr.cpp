/*
 * Copyright © 2018 Christian Persch
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "attr.h"

static inline void vte_attr_set_bool(uint32_t* attr,
                                     uint32_t mask,
                                     bool value)
{
        if (value)
                *attr |= mask;
        else
                *attr &= ~mask;
}

static inline void vte_attr_set_value(uint32_t* attr,
                                      uint32_t mask,
                                      unsigned int shift,
                                      uint32_t value)
{
        //g_assert_cmpuint(value << shift, <=, mask); /* assurance */
        *attr = (*attr & ~mask) | ((value << shift) & mask /* assurance */);
}

static constexpr inline bool vte_attr_get_bool(uint32_t attr,
                                               unsigned int shift)
{
        return (attr >> shift) & 1U;
}

static constexpr inline unsigned int vte_attr_get_value(uint32_t attr,
                                                        uint32_t value_mask,
                                                        unsigned int shift)
{
        return (attr >> shift) & value_mask;
}
