#include <stdlib.h>
#include <string.h>

#include "hmap.h"
#include "util.h"

struct _bodhi_hmap_t {
    bodhi_hash_fn hash_fn;
    bodhi_hmap_cmp_fn cmp_fn;
    bodhi_hmap_free_fn free_fn;

    size_t alloc_size;
    size_t consumed_size;

    void **keys;
    void **vals;
};

bodhi_hmap_t *bodhi_hmap_new_size(bodhi_hash_fn hash_fn, bodhi_hmap_cmp_fn cmp_fn,
                                  bodhi_hmap_free_fn key_free_fn, size_t size) {
    bodhi_hmap_t *ret;

    CALLOC(ret, 1, sizeof(bodhi_hmap_t), return NULL);
    CALLOC(ret->keys, size, sizeof(void*), return NULL);
    CALLOC(ret->vals, size, sizeof(void*), return NULL);
    ret->alloc_size = size;
    ret->consumed_size = 0;
    ret->hash_fn = hash_fn;
    ret->cmp_fn = cmp_fn;
    ret->free_fn = key_free_fn;

    return ret;
}

bodhi_hmap_t *bodhi_hmap_new(bodhi_hash_fn hash_fn, bodhi_hmap_cmp_fn cmp_fn, bodhi_hmap_free_fn key_free_fn) {
    return bodhi_hmap_new_size(hash_fn, cmp_fn, key_free_fn, 25);
}

static int _bodhi_hmap_resize(bodhi_hmap_t *hmap) {
    void **new_keys;
    void **new_vals;
    size_t new_size = hmap->alloc_size * 2;
    size_t iter;
    size_t index;

    CALLOC(new_keys, new_size, sizeof(void *), return -1);
    CALLOC(new_vals, new_size, sizeof(void *), return -1);

    for (iter = 0; iter < hmap->alloc_size; iter++) {
        if (hmap->keys[iter] == NULL) {
            continue;
        }

        index = hmap->hash_fn(hmap->keys[iter], new_size);
        new_keys[index] = hmap->keys[iter];
        new_vals[index] = hmap->vals[iter];
    }

    FREE(hmap->keys);
    FREE(hmap->vals);

    hmap->alloc_size = new_size;
    hmap->keys = new_keys;
    hmap->vals = new_vals;

    return 0;
}

int bodhi_hmap_insert(bodhi_hmap_t *hmap, void *key, size_t key_size, void *val, size_t val_size) {
    ASSERT(hmap != NULL, return -1);
    ASSERT(key != NULL, return -1);

    if (((double)(hmap->consumed_size) / (double)(hmap->alloc_size)) > 0.85) {
        /* TODO: If this fails, continue, but log that a resize failed */
        _bodhi_hmap_resize(hmap);
    }

    size_t index = hmap->hash_fn(key, hmap->alloc_size);
    if (index < 0 || index > hmap->alloc_size - 1) {
        return 0;
    }

    while (1) {
        if (hmap->keys[index] == NULL || hmap->keys[index] == key) {
            MALLOC(hmap->keys[index], key_size, return -1);
            memcpy(hmap->keys[index], key, key_size);
            break;
        } else {
            if (++index >= hmap->alloc_size) {
                index = 0;
            }
        }
    }
    MALLOC(hmap->vals[index], val_size, return -1);
    memcpy(hmap->vals[index], val, val_size);

    hmap->consumed_size++;
    return 1;
}

static size_t _bodhi_hmap_find_index(bodhi_hmap_t *hmap, void *kv) {
    ASSERT(kv != NULL, return hmap->alloc_size);
    size_t counter = 0;
    size_t start = hmap->hash_fn(kv, hmap->alloc_size);
    while (hmap->keys[start] != NULL && hmap->cmp_fn(kv, hmap->keys[start]) != 0 && counter < hmap->alloc_size) {
        if (++start >= hmap->alloc_size) {
            start = 0;
        }

        counter++;
    }

    if (counter >= hmap->alloc_size) {
        return hmap->alloc_size;
    }
    return start;
}

void *bodhi_hmap_delete(bodhi_hmap_t *hmap, void *key) {
    ASSERT(hmap != NULL, return NULL);

    size_t index = _bodhi_hmap_find_index(hmap, key);
    if (index == hmap->alloc_size) {
        return NULL;
    }

    void *ret = hmap->vals[index];

    hmap->free_fn(hmap->keys[index]);
    hmap->keys[index] = NULL;
    hmap->vals[index] = NULL;
    hmap->consumed_size--;

    return ret;
}

void *bodhi_hmap_value(bodhi_hmap_t *hmap, void *key) {
    ASSERT(hmap != NULL, return NULL);
    size_t index = _bodhi_hmap_find_index(hmap, key);
    if (index == hmap->alloc_size) {
        return NULL;
    }

    return hmap->vals[index];
}

size_t bodhi_hmap_size(bodhi_hmap_t *hmap) {
    ASSERT(hmap != NULL, return 0);
    return hmap->consumed_size;
}
