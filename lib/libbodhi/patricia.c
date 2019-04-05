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

#include <inttypes.h>
#include <stdlib.h>

#include "patricia.h"

struct _bodhi_patricia_t {
    struct _bodhi_patricia_t *left;
    struct _bodhi_patricia_t *right;
    struct _bodhi_patricia_t *parent;

    uint32_t key;
    int isset;
    int pos;
    void *data;
};

static bodhi_patricia_t *_alloc_bodhi_patricia() {
    bodhi_patricia_t *ret = calloc(1, sizeof(bodhi_patricia_t));
    return ret;
}

static unsigned int _shared_bits(uint32_t a, uint32_t b) {
    int i;
    for (i = 0; i <= 32; i++) {
        if (a >> i == b >> i) {
            return 32 - i;
        }
    }

    return 0;
}

bodhi_patricia_t *bodhi_patricia_new_blank() {
    return _alloc_bodhi_patricia();
}

bodhi_patricia_t *bodhi_patricia_new(uint32_t init_key, void *data) {
    bodhi_patricia_t *ret = _alloc_bodhi_patricia();

    ret->key = init_key;
    ret->data = data;
    ret->pos = 32;
    ret->isset = 1;

    return ret;
}

int bodhi_patricia_add(bodhi_patricia_t **trie_ptr, uint32_t key, void *data) {
    bodhi_patricia_t *trie = *trie_ptr;
    if (trie == NULL) {
        return 0;
    }

    /* is a duplicate record trying to be set? */
    if (trie->key == key && trie->isset) {
        /* perhaps log why this failed? */
        return 0;
    }

    unsigned int shared_bits = _shared_bits(key, trie->key);

    if (trie->pos > shared_bits) {
        bodhi_patricia_t *new = bodhi_patricia_new(key, data);

        bodhi_patricia_t *new_parent = _alloc_bodhi_patricia();
        new_parent->pos = shared_bits;

        if (shared_bits > 0) {
            new_parent->key = key & (0xFFFFFFFF << (32 - shared_bits));
        }
        new->parent = new_parent;

        /* this may be a bug */
        if ((trie->key & (0xFFFFFFFF << (32 - shared_bits - 1))) > new_parent->key) {
            new_parent->right = trie;
            new_parent->left = new;
        } else {
            new_parent->right = new;
            new_parent->left = trie;
        }

        new_parent->parent = trie->parent;
        trie->parent = new_parent;

        if (new_parent->parent == NULL) {
            *trie_ptr = new_parent;
        } else {
            if (new_parent->parent->left == trie) {
                new_parent->parent->left = new_parent;
            } else {
                new_parent->parent->right = new_parent;
            }
        }
    } else {
        /* check for children */
        if (trie->left != NULL) {
            if ((key & (1 << (31 - trie->pos))) == 0) {
                return bodhi_patricia_add(&trie->left, key, data);
            } else {
                return bodhi_patricia_add(&trie->right, key, data);
            }
        } else {
            /* this happens when blank */
            trie->key = key;
            trie->pos = 32;
            trie->data = data;
            trie->isset = 1;
        }
    }

    return 1;
}

int bodhi_patricia_remove(bodhi_patricia_t **trie_ptr, uint32_t key, void **retval) {
    bodhi_patricia_t *trie = *trie_ptr;
    int status;

    if (trie == NULL) {
        return 0;
    }

    if (!trie->isset) {
        status = bodhi_patricia_remove(&(trie->left), key, retval);
        if (!status) {
            status = bodhi_patricia_remove(&(trie->right), key, retval);
            return status;
        }
    }

    if (trie->key != key) {
        return 0;
    }

    if (trie->parent == NULL) {
        /* special case 1: we are removing a root node */
        *retval = trie->data;
        *trie_ptr = NULL;
        free(trie);
        return 1;
    }

    if (trie->parent->parent == NULL) {
        /* special case 2: we are removing a child of the root */
        if (trie == trie->parent->left) {
            *trie_ptr = trie->parent->right;
        } else {
            *trie_ptr = trie->parent->left;
        }

        (*trie_ptr)->parent = NULL;
        *retval = trie->data;
        free(trie);
        free(trie->parent);
        return 1;
    }

    /* now the most "common" situation */
    bodhi_patricia_t *sister;
    if (trie->parent->left == trie) {
        sister = trie->parent->right;
    } else {
        sister = trie->parent->left;
    }

    if (trie->parent->parent->left == trie->parent) {
        trie->parent->parent->left = sister;
    } else {
        trie->parent->parent->right = sister;
    }
    sister->parent = trie->parent->parent;

    *retval = trie->data;
    free(trie->parent);
    free(trie);
    return 1;
}

void bodhi_patricia_free(bodhi_patricia_t *trie, trie_free_fn fn) {
    while (1) {
        if (trie == NULL) {
            break;
        } else if (trie->left != NULL) {
            bodhi_patricia_free(trie->left, fn);
            trie->left = NULL;
        } else if (trie->right != NULL) {
            bodhi_patricia_free(trie->right, fn);
            trie->right = NULL;
        } else {
            fn(trie->data);
            free(trie);
            return;
        }
    }
}

void *bodhi_patricia_get_val(bodhi_patricia_t *trie, uint32_t key) {
    if (trie == NULL) {
        return NULL;
    }

    if (trie->isset && trie->key == key) {
        return trie->data;
    } else {
        if ((key & (1 << (31 - trie->pos))) == 0) {
            return bodhi_patricia_get_val(trie->left, key);
        } else {
            return bodhi_patricia_get_val(trie->right, key);
        }
    }
}

size_t bodhi_patricia_size(bodhi_patricia_t *trie) {
    if (trie->isset) {
        return 1;
    } else {
        return bodhi_patricia_size(trie->left) + bodhi_patricia_size(trie->right);
    }
}
