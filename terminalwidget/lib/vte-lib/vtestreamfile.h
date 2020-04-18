/*
 * Copyright (C) 2009,2010 Red Hat, Inc.
 * Copyright (C) 2013 Google, Inc.
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
 *
 * Red Hat Author(s): Behdad Esfahbod
 * Google Author(s): Behdad Esfahbod
 * Independent Author(s): Egmont Koblinger
 */

/*
 * VteFileStream is implemented as three layers above each other.
 *
 * o The bottom layer is VteSnake. It provides a mapping from logical offsets
 *   to physical file offsets, storing the stream in at most 3 continuous
 *   regions of the file. See below for details how this mapping is done.
 *
 *   It operates with a fixed block size (64kB at the moment), allows
 *   random-access-read of a single block, random-access-overwrite of a single
 *   block within the stream, write (append) a single block right after the
 *   current head, advancing the tail by arbitrary number of blocks, and
 *   resetting. The appended block can be shorter, in that case we still
 *   advance by 64kB and let the operating system leave a gap (sparse blocks)
 *   in the file which is crucial for compression.
 *
 *   (Random-access-overwrite within the existing area is a rare event, occurs
 *   only when the terminal window size changes. We use it to redo differently
 *   the latest appends. In the topmost layer it's achieved by truncating at
 *   the head of the stream, and then appending again. In this layer and also
 *   in the next one, offering random-access-overwrite instead of truncation
 *   makes the implementation a lot easier. It's also essential to maintain a
 *   unique encryption IV in the current design.)
 *
 *   The name was chosen because VteFileStream's way of advancing the head and
 *   the tail is kinda like a snake, and the mapping to file offsets reminds
 *   me of the well-known game on old mobile phones.
 *
 * o The middle layer is called VteBoa. It does compression and encryption
 *   along with integrity check. It has (almost) the same API as the snake,
 *   but the blocksize is a bit smaller to leave room for the required
 *   overhead. See below for the exact layout.
 *
 *   This overhead consists of the length of the compressed+encrypted payload,
 *   an overwrite counter, and the encryption verification tag. The overwrite
 *   counter is incremented each time the data at a certain logical offset is
 *   overwritten, this is used in constructing a unique IV.
 *
 *   The name was chosen because the world of encryption is full of three
 *   letter abbreviations. At this moment we use GNU TLS's method for doing
 *   AES GCM. Also, because grown-ups might think it's a hat, when actually
 *   it's a boa constrictor digesting an elephant :)
 *
 * o The top layer is VteFileStream. It does buffering and caching. As opposed
 *   to the previous layers, this one provides methods on arbitrary amount of
 *   data. It doesn't offer random-access-writes, instead, it offers appending
 *   data, and truncating the head (undoing the latest appends). Write
 *   requests are batched up until there's a complete block to be compressed,
 *   encrypted and written to disk. Read requests are answered by reading,
 *   decrypting and uncompressing possibly more underlying blocks, and sped up
 *   by caching the result.
 *
 * Design discussions: https://bugzilla.gnome.org/show_bug.cgi?id=738601
 */

#pragma once

#include <glib.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <zlib.h>
#include <glib-object.h>
#include <gio/gio.h>

#ifdef WITH_GNUTLS
# include <gnutls/gnutls.h>
# include <gnutls/crypto.h>
#endif

//#include "vteutils.h"

#ifdef WITH_GNUTLS
/* Currently the code requires that a stream cipher (e.g. GCM) is used
 * which can encrypt any amount of data without need for padding. */
# define VTE_CIPHER_ALGORITHM    GNUTLS_CIPHER_AES_256_GCM
# define VTE_CIPHER_KEY_SIZE     32
# define VTE_CIPHER_IV_SIZE      12
# define VTE_CIPHER_TAG_SIZE     16
#else
# define VTE_CIPHER_TAG_SIZE     0
#endif

#ifndef VTESTREAM_MAIN
# define VTE_SNAKE_BLOCKSIZE 65536
typedef guint32 _vte_block_datalength_t;
typedef guint32 _vte_overwrite_counter_t;
#else
/* Smaller sizes for unit testing */
# define VTE_SNAKE_BLOCKSIZE    10
typedef guint8 _vte_block_datalength_t;
typedef guint8 _vte_overwrite_counter_t;
# undef VTE_CIPHER_TAG_SIZE
# define VTE_CIPHER_TAG_SIZE     1
#endif

#define VTE_BLOCK_DATALENGTH_SIZE  sizeof(_vte_block_datalength_t)
#define VTE_OVERWRITE_COUNTER_SIZE sizeof(_vte_overwrite_counter_t)
#define VTE_BOA_BLOCKSIZE (VTE_SNAKE_BLOCKSIZE - VTE_BLOCK_DATALENGTH_SIZE - VTE_OVERWRITE_COUNTER_SIZE - VTE_CIPHER_TAG_SIZE)

#define OFFSET_BOA_TO_SNAKE(x) ((x) / VTE_BOA_BLOCKSIZE * VTE_SNAKE_BLOCKSIZE)
#define ALIGN_BOA(x) ((x) / VTE_BOA_BLOCKSIZE * VTE_BOA_BLOCKSIZE)
#define MOD_BOA(x)   ((x) % VTE_BOA_BLOCKSIZE)

/******************************************************************************************/

#ifndef HAVE_EXPLICIT_BZERO
#define explicit_bzero(s, n) memset((s), 0, (n))
#endif

/*
 * VteStream: Abstract base stream class
 */

typedef struct _VteStream {
    GObject parent;
} VteStream;

void _vte_stream_reset (VteStream *stream, gsize offset);
gboolean _vte_stream_read (VteStream *stream, gsize offset, char *data, gsize len);
void _vte_stream_append (VteStream *stream, const char *data, gsize len);
void _vte_stream_truncate (VteStream *stream, gsize offset);
void _vte_stream_advance_tail (VteStream *stream, gsize offset);
gsize _vte_stream_tail (VteStream *stream);
gsize _vte_stream_head (VteStream *stream);

/* Various streams */

VteStream *
_vte_file_stream_new (void);

typedef struct _VteStreamClass {
    GObjectClass parent_class;

    void (*reset) (VteStream *stream, gsize offset);
    gboolean (*read) (VteStream *stream, gsize offset, char *data, gsize len);
    void (*append) (VteStream *stream, const char *data, gsize len);
    void (*truncate) (VteStream *stream, gsize offset);
    void (*advance_tail) (VteStream *stream, gsize offset);
    gsize (*tail) (VteStream *stream);
    gsize (*head) (VteStream *stream);
} VteStreamClass;

static GType _vte_stream_get_type (void);
#define VTE_TYPE_STREAM _vte_stream_get_type ()
#define VTE_STREAM_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), VTE_TYPE_STREAM, VteStreamClass))

G_DEFINE_ABSTRACT_TYPE (VteStream, _vte_stream, G_TYPE_OBJECT)

static void
_vte_stream_class_init (VteStreamClass *klass G_GNUC_UNUSED){}

static void
_vte_stream_init (VteStream *stream G_GNUC_UNUSED){}

void
_vte_stream_reset (VteStream *stream, gsize offset);

gboolean
_vte_stream_read (VteStream *stream, gsize offset, char *data, gsize len);

void
_vte_stream_append (VteStream *stream, const char *data, gsize len);

void
_vte_stream_truncate (VteStream *stream, gsize offset);

void
_vte_stream_advance_tail (VteStream *stream, gsize offset);

gsize
_vte_stream_tail (VteStream *stream);

gsize
_vte_stream_head (VteStream *stream);
