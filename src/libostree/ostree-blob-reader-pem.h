/*
 * Copyright (C) 2024 Red Hat, Inc.
 *
 * SPDX-License-Identifier: LGPL-2.0+
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
 * License along with this library. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "ostree-blob-reader.h"

G_BEGIN_DECLS

#define OSTREE_TYPE_BLOB_READER_PEM (_ostree_blob_reader_pem_get_type ())

_OSTREE_PUBLIC
G_DECLARE_FINAL_TYPE (OstreeBlobReaderPem, _ostree_blob_reader_pem, OSTREE, BLOB_READER_PEM,
                      GDataInputStream);

_OSTREE_PUBLIC
OstreeBlobReaderPem *_ostree_blob_reader_pem_new (GInputStream *base, const gchar *label);

_OSTREE_PUBLIC
GBytes *ostree_blob_reader_pem_read_blob (OstreeBlobReader *self, GCancellable *cancellable,
                                          GError **error);

G_END_DECLS
