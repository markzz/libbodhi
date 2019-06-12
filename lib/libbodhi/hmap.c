/*
 * hmap.c
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

#include <stdlib.h>
#include <string.h>

#include "hmap.h"
#include "util.h"
#include "list.h"

typedef struct _bodhi_hmap_bucket_t {
    size_t h;
    void *key;
    void *val;
} bodhi_hmap_bucket_t;

struct _bodhi_hmap_t {
    bodhi_hash_fn hash_fn;
    bodhi_hmap_cmp_fn cmp_fn;
    bodhi_hmap_free_fn key_free_fn;
    bodhi_hmap_free_fn val_free_fn;

    size_t alloc_size;
    size_t consumed_size;

    bodhi_list_t **buckets;
};

static void _bodhi_hmap_bucket_free_inner(bodhi_hmap_t *hmap, bodhi_list_t *buckets) {
    bodhi_list_t *tmp;

    for (tmp = buckets; tmp; tmp = tmp->next) {
        bodhi_hmap_bucket_t *bucket = tmp->data;
        if (bucket->key != NULL) {
            hmap->key_free_fn(bucket->key);
        }

        if (bucket->val != NULL) {
            hmap->val_free_fn(bucket->val);
        }
    }

    FREELIST(buckets);
}

static void _bodhi_hmap_bucket_free(bodhi_hmap_t *hmap) {
    int i;

    for (i = 0; i < hmap->alloc_size; i++) {
        if (hmap->buckets[i] != NULL) {
            _bodhi_hmap_bucket_free_inner(hmap, hmap->buckets[i]);
        }
    }
}

bodhi_hmap_t *bodhi_hmap_new_size(bodhi_hash_fn hash_fn, bodhi_hmap_cmp_fn cmp_fn,
                                  bodhi_hmap_free_fn key_free_fn,
                                  bodhi_hmap_free_fn val_free_fn, size_t size) {
    bodhi_hmap_t *ret;

    CALLOC(ret, 1, sizeof(bodhi_hmap_t), return NULL);
    CALLOC(ret->buckets, size, sizeof(bodhi_list_t*), return NULL);
    ret->alloc_size = size;
    ret->consumed_size = 0;
    ret->hash_fn = hash_fn;
    ret->cmp_fn = cmp_fn;
    ret->key_free_fn = key_free_fn;
    ret->val_free_fn = val_free_fn;

    return ret;
}

bodhi_hmap_t *bodhi_hmap_new(bodhi_hash_fn hash_fn, bodhi_hmap_cmp_fn cmp_fn,
                             bodhi_hmap_free_fn key_free_fn, bodhi_hmap_free_fn val_free_fn) {
    return bodhi_hmap_new_size(hash_fn, cmp_fn, key_free_fn, val_free_fn, 32);
}

void bodhi_hmap_free(bodhi_hmap_t *hmap) {
    ASSERT(hmap != NULL, return);
    _bodhi_hmap_bucket_free(hmap);
    free(hmap->buckets);
    free(hmap);
}

static int _bodhi_hmap_resize(bodhi_hmap_t *hmap) {
    bodhi_list_t **bkts;
    size_t new_size = hmap->alloc_size * 2;
    size_t iter;

    bkts = realloc(hmap->buckets, sizeof(bodhi_list_t*) * new_size);
    if (bkts == NULL) {
        return -1;
    }

    for (iter = hmap->alloc_size; iter < new_size; iter++) {
        bkts[iter] = NULL;
    }

    for (iter = 0; iter < hmap->alloc_size; iter++) {
        bodhi_list_t *tmp = bkts[iter];
        while (tmp != NULL) {
            bodhi_list_t *next = tmp->next;
            bodhi_hmap_bucket_t *b = tmp->data;
            size_t new_ind = b->h & (new_size - 1);
            if (new_ind == iter) {
                tmp = next;
                continue;
            }

            bkts[iter] = bodhi_list_remove_item(bkts[iter], tmp);
            bkts[new_ind] = bodhi_list_join(bkts[new_ind], tmp);
            tmp = next;
        }
    }

    hmap->alloc_size = new_size;
    hmap->buckets = bkts;

    return 0;
}

static int _bodhi_hmap_key_cmp(bodhi_hmap_cmp_fn fn, void *a, void *b) {
    return fn(a, b);
}

int bodhi_hmap_insert_no_cpy(bodhi_hmap_t *hmap, void *key, void *val) {
    ASSERT(hmap != NULL, return -1);
    ASSERT(key != NULL, return -1);
    bodhi_hmap_bucket_t *bkt = NULL;
    bodhi_list_t *list;

    if (hmap->consumed_size == hmap->alloc_size) {
        /* TODO: If this fails, continue, but log that a resize failed */
        _bodhi_hmap_resize(hmap);
    }

    size_t hash = hmap->hash_fn(key);
    size_t index = hash & (hmap->alloc_size - 1);
    if (index >= hmap->alloc_size) {
        return -1;
    }

    for (list = hmap->buckets[index]; list; list = list->next) {
        bkt = list->data;

        if (bkt->h == hash && hmap->cmp_fn(bkt->key, key) == 0) {
            return 1;
        }
    }

    CALLOC(bkt, 1, sizeof(bodhi_hmap_bucket_t), return -1);
    bkt->h = hash;
    bkt->key = key;
    bkt->val = val;

    if (hmap->buckets[index] == NULL) {
        hmap->buckets[index] = bodhi_list_new(bkt);
    } else {
        bodhi_list_add(hmap->buckets[index], bkt);
    }

    hmap->consumed_size++;

    return 0;
}

int bodhi_hmap_insert(bodhi_hmap_t *hmap, void *key, size_t key_size, void *val, size_t val_size) {
    ASSERT(hmap != NULL, return -1);
    ASSERT(key != NULL, return -1);

    int res;
    void *key_cpy = NULL;
    void *val_cpy = NULL;

    CALLOC(key_cpy, key_size, sizeof(void), return -1);
    CALLOC(val_cpy, val_size, sizeof(void), return -1);
    memcpy(key_cpy, key, key_size);
    memcpy(val_cpy, val, val_size);

    res = bodhi_hmap_insert_no_cpy(hmap, key_cpy, val_cpy);

    if (res != 0) {
        free(key_cpy);
        free(val_cpy);
        return res;
    } else {
        return 0;
    }
}

static size_t _bodhi_hmap_find_index(bodhi_hmap_t *hmap, void *kv) {
    ASSERT(kv != NULL, return hmap->alloc_size);
    size_t index = hmap->hash_fn(kv) % hmap->alloc_size;
    return index;
}

static bodhi_list_t *_bodhi_hmap_get_list_item(bodhi_hmap_t *hmap, void *key) {
    ASSERT(key != NULL, return NULL);
    bodhi_list_t *tmp;
    size_t index = _bodhi_hmap_find_index(hmap, key);
    if (index == hmap->alloc_size) {
        return NULL;
    }

    for (tmp = hmap->buckets[index]; tmp; tmp = tmp->next) {
        bodhi_hmap_bucket_t *bkt = tmp->data;
        if (hmap->cmp_fn(key, bkt->key) == 0) {
            return tmp;
        }
    }

    return NULL;
}

int bodhi_hmap_delete(bodhi_hmap_t *hmap, void *key) {
    ASSERT(hmap != NULL, return -1);

    bodhi_list_t *to_rm;
    size_t index = _bodhi_hmap_find_index(hmap, key);

    if (index == hmap->alloc_size) {
        return -1;
    }

    to_rm = _bodhi_hmap_get_list_item(hmap, key);
    if (to_rm != NULL) {
        bodhi_list_remove_item(hmap->buckets[index], to_rm);
        hmap->consumed_size--;
        return 0;
    }

    return 1;

}

int bodhi_hmap_key_exists(bodhi_hmap_t *hmap, void *key) {
    ASSERT(hmap != NULL, return -1);

    if (_bodhi_hmap_get_list_item(hmap, key) != NULL) {
        return 0;
    }

    return 1;
}

void *bodhi_hmap_value(bodhi_hmap_t *hmap, void *key) {
    ASSERT(hmap != NULL, return NULL);
    bodhi_list_t *item = _bodhi_hmap_get_list_item(hmap, key);

    if (item == NULL) {
        return NULL;
    }

    bodhi_hmap_bucket_t *bkt = item->data;
    return bkt->val;
}

size_t bodhi_hmap_size(bodhi_hmap_t *hmap) {
    ASSERT(hmap != NULL, return 0);
    return hmap->consumed_size;
}

bodhi_list_t *bodhi_hmap_get_keys(bodhi_hmap_t *hmap) {
    ASSERT(hmap != NULL, return NULL);

    bodhi_list_t *ret = NULL;
    size_t cur;

    for (cur = 0; cur < hmap->alloc_size; cur++) {
        if (hmap->buckets[cur] != NULL) {
            bodhi_list_t *tmp;
            for (tmp = hmap->buckets[cur]; tmp; tmp = tmp->next) {
                bodhi_hmap_bucket_t *bkt = tmp->data;
                if (ret == NULL) {
                    ret = bodhi_list_new(bkt->key);
                } else {
                    bodhi_list_add(ret, bkt->key);
                }
            }
        }
    }

    return ret;
}

bodhi_list_t *bodhi_hmap_get_keyvals(bodhi_hmap_t *hmap) {
    ASSERT(hmap != NULL, return NULL);

    bodhi_list_t *ret = NULL;
    size_t cur;

    for (cur = 0; cur < hmap->alloc_size; cur++) {
        if (hmap->buckets[cur] != NULL) {
            bodhi_list_t *tmp;
            for (tmp = hmap->buckets[cur]; tmp; tmp = tmp->next) {
                bodhi_hmap_keyval_t *val = tmp->data + sizeof(size_t);
                if (ret == NULL) {
                    ret = bodhi_list_new(val);
                } else {
                    bodhi_list_add(ret, val);
                }
            }
        }
    }

    return ret;
}