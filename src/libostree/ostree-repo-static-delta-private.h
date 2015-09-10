/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*-
 *
 * Copyright (C) 2013 Colin Walters <walters@verbum.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#pragma once

#include "ostree-core.h"

G_BEGIN_DECLS

/* Arbitrarily chosen */
#define OSTREE_STATIC_DELTA_PART_MAX_SIZE_BYTES (16*1024*1024)
/* 1 byte for object type, 32 bytes for checksum */
#define OSTREE_STATIC_DELTA_OBJTYPE_CSUM_LEN 33

#define OSTREE_SUMMARY_STATIC_DELTAS "ostree.static-deltas"

/**
 * OSTREE_STATIC_DELTA_PART_PAYLOAD_FORMAT_V0:
 *
 *   y  compression type (0: none, 'x': lzma)
 *   ---
 *   a(uuu) modes
 *   aa(ayay) xattrs
 *   ay raw data source
 *   ay operations
 */
#define OSTREE_STATIC_DELTA_PART_PAYLOAD_FORMAT_V0 "(a(uuu)aa(ayay)ayay)"

/**
 * OSTREE_STATIC_DELTA_META_ENTRY_FORMAT:
 *
 *   u: version
 *   ay checksum
 *   guint64 size:   Total size of delta (sum of parts)
 *   guint64 usize:   Uncompressed size of resulting objects on disk
 *   ARRAY[(guint8 objtype, csum object)]
 *
 * The checksum is of the delta payload, and each entry in the array
 * represents an OSTree object which will be created by the deltapart.
 */

#define OSTREE_STATIC_DELTA_META_ENTRY_FORMAT "(uayttay)"


/**
 * OSTREE_STATIC_DELTA_FALLBACK_FORMAT:
 *
 * y: objtype
 * ay: checksum
 * t: compressed size
 * t: uncompressed size
 *
 * Object to fetch invididually; includes compressed/uncompressed size.
 */
#define OSTREE_STATIC_DELTA_FALLBACK_FORMAT "(yaytt)"

/**
 * OSTREE_STATIC_DELTA_SUPERBLOCK_FORMAT:
 *
 * A .delta object is a custom binary format.  It has the following high
 * level form:
 *
 * delta-descriptor:
 *   metadata: a{sv}
 *   t: timestamp
 *   from: ay checksum
 *   to: ay checksum
 *   commit: new commit object
 *   ARRAY[(csum from, csum to)]: ay
 *   ARRAY[delta-meta-entry]
 *   array[fallback]
 *
 * The metadata would include things like a version number, as well as
 * extended verification data like a GPG signature.
 * 
 * The second array is an array of delta objects that should be
 * fetched and applied before this one.  This is a fairly generic
 * recursion mechanism that would potentially allow saving significant
 * storage space on the server.
 *
 * The heart of the static delta: the array of delta parts.
 *
 * Finally, we have the fallback array, which is the set of objects to
 * fetch individually - the compiler determined it wasn't worth
 * duplicating the space.
 */ 
#define OSTREE_STATIC_DELTA_SUPERBLOCK_FORMAT "(a{sv}tayay" OSTREE_COMMIT_GVARIANT_STRING "aya" OSTREE_STATIC_DELTA_META_ENTRY_FORMAT "a" OSTREE_STATIC_DELTA_FALLBACK_FORMAT ")"

gboolean _ostree_static_delta_part_validate (OstreeRepo     *repo,
                                             GInputStream   *in,
                                             guint           part_offset,
                                             const char     *expected_checksum,
                                             GCancellable   *cancellable,
                                             GError        **error);

gboolean _ostree_static_delta_part_execute (OstreeRepo      *repo,
                                            GVariant        *header,
                                            GBytes          *partdata,
                                            GCancellable    *cancellable,
                                            GError         **error);

gboolean _ostree_static_delta_part_execute_raw (OstreeRepo      *repo,
                                                GVariant        *header,
                                                GVariant        *part,
                                                GCancellable    *cancellable,
                                                GError         **error);

void _ostree_static_delta_part_execute_async (OstreeRepo      *repo,
                                              GVariant        *header,
                                              GBytes          *partdata,
                                              GCancellable    *cancellable,
                                              GAsyncReadyCallback  callback,
                                              gpointer         user_data);

gboolean _ostree_static_delta_part_execute_finish (OstreeRepo      *repo,
                                                   GAsyncResult    *result,
                                                   GError         **error); 

typedef enum {
  OSTREE_STATIC_DELTA_OP_OPEN_SPLICE_AND_CLOSE = 'S',
  OSTREE_STATIC_DELTA_OP_OPEN = 'o',
  OSTREE_STATIC_DELTA_OP_WRITE = 'w',
  OSTREE_STATIC_DELTA_OP_SET_READ_SOURCE = 'r',
  OSTREE_STATIC_DELTA_OP_UNSET_READ_SOURCE = 'R',
  OSTREE_STATIC_DELTA_OP_CLOSE = 'c',
  OSTREE_STATIC_DELTA_OP_BSPATCH = 'B'
} OstreeStaticDeltaOpCode;

gboolean
_ostree_static_delta_parse_checksum_array (GVariant      *array,
                                           guint8       **out_checksums_array,
                                           guint         *out_n_checksums,
                                           GError       **error);

gboolean
_ostree_repo_static_delta_part_have_all_objects (OstreeRepo             *repo,
                                                 GVariant               *checksum_array,
                                                 gboolean               *out_have_all,
                                                 GCancellable           *cancellable,
                                                 GError                **error);

typedef struct {
  char *checksum;
  guint64 size;
  GPtrArray *basenames;
} OstreeDeltaContentSizeNames;

void _ostree_delta_content_sizenames_free (gpointer v);

gboolean
_ostree_delta_compute_similar_objects (OstreeRepo                 *repo,
                                       GVariant                   *from_commit,
                                       GVariant                   *to_commit,
                                       GHashTable                 *new_reachable_regfile_content,
                                       guint                       similarity_percent_threshold,
                                       GHashTable                **out_modified_regfile_content,
                                       GCancellable               *cancellable,
                                       GError                    **error);

G_END_DECLS
