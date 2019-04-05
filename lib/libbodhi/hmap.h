/*
 * hmap.h
 *
 * Copyright (c) 2018, Mark Weiman <mark.weiman@markzz.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef BODHI_HMAP_H
#define BODHI_HMAP_H

#include <stdlib.h>

#include <bodhi/list.h>

typedef struct _bodhi_hmap_t bodhi_hmap_t;

typedef size_t (*bodhi_hash_fn)(void *, size_t);
typedef int (*bodhi_hmap_cmp_fn)(const void *, const void *);
typedef void (*bodhi_hmap_free_fn)(void *);

bodhi_hmap_t *bodhi_hmap_new_size(bodhi_hash_fn hash_fn, bodhi_hmap_cmp_fn cmp_fn,
    bodhi_hmap_free_fn key_free_fn, size_t size);
bodhi_hmap_t *bodhi_hmap_new(bodhi_hash_fn hash_fn, bodhi_hmap_cmp_fn cmp_fn, bodhi_hmap_free_fn key_free_fn);
void bodhi_hmap_free(bodhi_hmap_t *hmap, bodhi_hmap_free_fn val_free);
int bodhi_hmap_insert_no_cpy(bodhi_hmap_t *hmap, void *key, void *val);
int bodhi_hmap_insert(bodhi_hmap_t *hmap, void *key, size_t key_size, void *val, size_t val_size);
void *bodhi_hmap_delete(bodhi_hmap_t *hmap, void *key);
int bodhi_hmap_key_exists(bodhi_hmap_t *hmap, void *key);
void *bodhi_hmap_value(bodhi_hmap_t *hmap, void *key);
size_t bodhi_hmap_size(bodhi_hmap_t *hmap);
bodhi_list_t *bodhi_hmap_get_keys(bodhi_hmap_t *hmap);

#endif
