#ifndef BODHI_HMAP_H
#define BODHI_HMAP_H

#include <stdlib.h>

typedef struct _bodhi_hmap_t bodhi_hmap_t;

typedef size_t (*bodhi_hash_fn)(void *, size_t);
typedef int (*bodhi_hmap_cmp_fn)(void *, void *);
typedef void (*bodhi_hmap_free_fn)(void *);

bodhi_hmap_t *bodhi_hmap_new_size(bodhi_hash_fn hash_fn, bodhi_hmap_cmp_fn cmp_fn,
    bodhi_hmap_free_fn key_free_fn, size_t size);
bodhi_hmap_t *bodhi_hmap_new(bodhi_hash_fn hash_fn, bodhi_hmap_cmp_fn cmp_fn, bodhi_hmap_free_fn key_free_fn);
int bodhi_hmap_insert(bodhi_hmap_t *hmap, void *key, size_t key_size, void *val, size_t val_size);
void *bodhi_hmap_delete(bodhi_hmap_t *hmap, void *key);
void *bodhi_hmap_value(bodhi_hmap_t *hmap, void *key);


#endif
