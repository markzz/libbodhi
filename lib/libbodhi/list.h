/*
 * list.h
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

#ifndef BODHI_LIST_H
#define BODHI_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

typedef struct _bodhi_list_t {
    void *data;
    struct _bodhi_list_t *prev;
    struct _bodhi_list_t *next;
} bodhi_list_t;

#define FREELIST(p) do { bodhi_list_free_inner(p, free); bodhi_list_free(p); p = NULL; } while (0)

typedef void (*bodhi_list_free_fn)(void *);
typedef int (*bodhi_list_cmp_fn)(const void *, const void *);

void bodhi_list_free(bodhi_list_t *list);
void bodhi_list_free_inner(bodhi_list_t *list, bodhi_list_free_fn fn);
bodhi_list_t *bodhi_list_new(void *data);
bodhi_list_t *bodhi_list_add(bodhi_list_t *list, void *data);
bodhi_list_t *bodhi_list_add_sorted(bodhi_list_t *list, void *data, bodhi_list_cmp_fn fn);
bodhi_list_t *bodhi_list_join(bodhi_list_t *left, bodhi_list_t *right);
bodhi_list_t *bodhi_list_mmerge(bodhi_list_t *left, bodhi_list_t *right, bodhi_list_cmp_fn fn);
bodhi_list_t *bodhi_list_msort(bodhi_list_t *list, bodhi_list_cmp_fn fn);
bodhi_list_t *bodhi_list_remove_item(bodhi_list_t *list, bodhi_list_t *item);
bodhi_list_t *bodhi_list_remove(bodhi_list_t *list, const void *needle, bodhi_list_cmp_fn fn, void **data);
bodhi_list_t *bodhi_list_remove_dupes(const bodhi_list_t *list);
bodhi_list_t *bodhi_list_copy(const bodhi_list_t *list);
bodhi_list_t *bodhi_list_copy_data(const bodhi_list_t *list, size_t size);
bodhi_list_t *bodhi_list_reverse(bodhi_list_t *list);

bodhi_list_t *bodhi_list_nth(bodhi_list_t *list, size_t n);
bodhi_list_t *bodhi_list_last(bodhi_list_t *list);

size_t bodhi_list_count(bodhi_list_t *list);
void *bodhi_list_find(const bodhi_list_t *list, const void *needle, bodhi_list_cmp_fn fn);
void **bodhi_list_to_array(bodhi_list_t *list, size_t size);

#ifdef __cplusplus
}
#endif

#endif
