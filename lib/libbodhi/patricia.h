/*
 * patricia.c
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

#ifndef BODHI_PATRICIA_H
#define BODHI_PATRICIA_H

#include <inttypes.h>

typedef struct _bodhi_patricia_t bodhi_patricia_t;

typedef void (trie_free_fn)(void*);

void bodhi_patricia_free(bodhi_patricia_t *trie, trie_free_fn fn);
bodhi_patricia_t *bodhi_patricia_new_blank();
bodhi_patricia_t *bodhi_patricia_new(uint32_t init_key, void *data);
int bodhi_patricia_add(bodhi_patricia_t **trie, uint32_t key, void *data);
int bodhi_patricia_remove(bodhi_patricia_t **trie, uint32_t key, void **retval);
void *bodhi_patricia_get_val(bodhi_patricia_t *trie, uint32_t key);
size_t bodhi_patricia_size(bodhi_patricia_t *trie);

#endif
