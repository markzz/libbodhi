/*
 * list.c
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

#include <string.h>

#include "list.h"
#include "util.h"

void bodhi_list_free(bodhi_list_t *list) {
    bodhi_list_t *iter;
    bodhi_list_t *tmp;

    for (iter = list; iter; iter = tmp) {
        tmp = iter->next;
        free(iter);
    }
}

void bodhi_list_free_inner(bodhi_list_t *list, bodhi_list_free_fn fn) {
    bodhi_list_t *iter;

    if (fn != NULL) {
        for (iter = list; iter; iter = iter->next) {
            if (iter->data != NULL) {
                fn(iter->data);
            }
        }
    }
}

bodhi_list_t *bodhi_list_new(void *data) {
    bodhi_list_t *ret = NULL;

    CALLOC(ret, 1, sizeof(bodhi_list_t), return NULL);

    ret->data = data;
    ret->next = NULL;
    ret->prev = ret;

    return ret;
}

bodhi_list_t *bodhi_list_add(bodhi_list_t *list, void *data) {
    bodhi_list_t *new = NULL;
    bodhi_list_t *last;

    CALLOC(new, 1, sizeof(bodhi_list_t), return list);

    new->next = NULL;
    new->data = data;

    if (list != NULL) {
        last = list->prev;
        last->next = new;
        new->prev = last;
        list->prev = new;
        return list;
    } else {
        return new;
    }
}

bodhi_list_t *bodhi_list_add_sorted(bodhi_list_t *list, void *data, bodhi_list_cmp_fn fn) {
    if (fn == NULL || list == NULL) {
        return bodhi_list_add(list, data);
    } else {
        bodhi_list_t *new = NULL;
        bodhi_list_t *next = list;
        bodhi_list_t *prev = NULL;

        MALLOC(new, sizeof(bodhi_list_t), return list);
        new->data = data;

        while (next != NULL) {
            if (fn(new->data, next->data) <= 0) {
                break;
            }

            prev = next;
            next = next->next;
        }

        if (prev == NULL) {
            new->prev = list->prev;
            new->next = next;
            list->prev = new;
            return new;
        } else if (next == NULL) {
            new->prev = prev;
            new->next = NULL;
            prev->next = new;
            list->prev = new;
            return list;
        } else {
            new->prev = prev;
            new->next = next;
            next->prev = new;
            prev->next = new;
            return list;
        }
    }
}

bodhi_list_t *bodhi_list_join(bodhi_list_t *left, bodhi_list_t *right) {
    bodhi_list_t *tmp;

    if (left == NULL) {
        return right;
    }

    if (right == NULL) {
        return left;
    }

    tmp = left->prev;
    tmp->next = right;
    left->prev = right->prev;
    right->prev = tmp;

    return left;
}

bodhi_list_t *bodhi_list_mmerge(bodhi_list_t *left, bodhi_list_t *right, bodhi_list_cmp_fn fn) {
    bodhi_list_t *new;
    bodhi_list_t *lp;
    bodhi_list_t *last;
    bodhi_list_t *left_last;
    bodhi_list_t *right_last;

    if (left == NULL) {
        return right;
    }

    if (right == NULL) {
        return left;
    }

    left_last = left->prev;
    right_last = right->prev;

    if (fn(left->data, right->data) <= 0) {
        new = left;
        left = left->next;
    } else {
        new = right;
        right = right->next;
    }
    new->prev = NULL;
    new->next = NULL;
    lp = new;

    while ((left != NULL) && (right != NULL)) {
        if (fn(left->data, right->data) <= 0) {
            lp->next = left;
            left->prev = lp;
            left = left->next;
        } else {
            lp->next = right;
            right->prev = lp;
            right = right->next;
        }

        lp = lp->next;
        lp->next = NULL;
    }

    if (left != NULL) {
        lp->next = left;
        left->prev = lp;
        last = left_last;
    } else if (right != NULL) {
        lp->next = right;
        right->prev = lp;
        last = right_last;
    } else {
        last = lp;
    }

    new->prev = last;

    return new;
}

bodhi_list_t *bodhi_list_msort(bodhi_list_t *list, bodhi_list_cmp_fn fn) {
    size_t count = bodhi_list_count(list);
    size_t i;
    size_t h;
    bodhi_list_t *left = list;
    bodhi_list_t *left_last = list;
    bodhi_list_t *right;

    if (count > 1) {
        h = count / 2;

        for (i = h - 1; i > 0; i--) {
            left_last = left_last->next;
        }
        right = left_last->next;

        left_last->next = NULL;
        right->prev = left->prev;
        left->prev = left_last;

        left = bodhi_list_msort(left, fn);
        right = bodhi_list_msort(right, fn);
        list = bodhi_list_mmerge(left, right, fn);
    }

    return list;
}

bodhi_list_t *bodhi_list_remove_item(bodhi_list_t *list, bodhi_list_t *item) {
    if (list == NULL || item == NULL) {
        return list;
    }

    if (list == item) {
        list = list->next;

        if (list != NULL) {
            list->prev = item->prev;
        }
        item->next = NULL;
    } else if (item == list->prev) {
        item->prev->next = NULL;
        list->prev = item->prev;
    } else {
        if (item->next != NULL) {
            item->next->prev = item->prev;
        }

        if (item->prev != NULL) {
            item->prev->next = item->next;
        }
        item->next = NULL;
    }

    item->prev = item;
    return list;
}

bodhi_list_t *bodhi_list_remove(bodhi_list_t *list, const void *needle, bodhi_list_cmp_fn fn, void **data) {
    bodhi_list_t *tmp;

    if (data != NULL) {
        *data = NULL;
    }

    if (needle == NULL) {
        return list;
    }

    for (tmp = list; tmp != NULL; tmp = tmp->next) {
        if (tmp->data == NULL) {
            continue;
        }

        if (fn(tmp->data, needle) == 0) {
            list = bodhi_list_remove_item(list, tmp);


            if (data != NULL) {
                *data = tmp->data;
            }

            free(tmp);
            break;
        }
    }

    return list;
}

static cmp_ptr(const void *a, const void *b) {
    return a == b;
}

bodhi_list_t *bodhi_list_remove_dupes(const bodhi_list_t *list) {
    const bodhi_list_t *tmp;
    bodhi_list_t *ret = NULL;

    for (tmp = list; tmp; tmp = tmp->next) {
        if (bodhi_list_find(ret, tmp->data, cmp_ptr) == NULL) {
            if (ret == NULL) {
                if ((ret = bodhi_list_new(tmp->data)) == NULL) {
                    return NULL;
                }
            } else {
                if (bodhi_list_add(ret, tmp->data) == NULL) {
                    bodhi_list_free(ret);
                    return NULL;
                }
            }
        }
    }

    return ret;
}

bodhi_list_t *bodhi_list_copy(const bodhi_list_t *list) {
    const bodhi_list_t *iter;
    bodhi_list_t *ret = NULL;

    for (iter = list; iter; iter = iter->next) {
        if (ret == NULL) {
            if ((ret = bodhi_list_new(iter->data)) == NULL) {
                return NULL;
            }
        } else {
            if (bodhi_list_add(ret, iter->data) == NULL) {
                bodhi_list_free(ret);
                return NULL;
            }
        }
    }

    return ret;
}

bodhi_list_t *bodhi_list_copy_data(const bodhi_list_t *list, size_t size) {
    const bodhi_list_t *iter;
    bodhi_list_t *ret = NULL;
    void *data;

    for (iter = list; iter; iter = iter->next) {
        data = malloc(size);
        if (data == NULL) {
            memcpy(data, iter->data, size);
            if (ret == NULL) {
                if ((ret = bodhi_list_new(iter->data)) == NULL) {
                    free(data);
                    FREELIST(ret);
                    return NULL;
                }
            } else {
                if (bodhi_list_add(ret, iter->data) == NULL) {
                    bodhi_list_free(ret);
                    return NULL;
                }
            }
        } else {
            FREELIST(ret);
            return NULL;
        }
    }

    return ret;
}

bodhi_list_t *bodhi_list_reverse(bodhi_list_t *list) {
    const bodhi_list_t *iter;
    bodhi_list_t *ret = NULL;
    bodhi_list_t *tmp;

    if (list == NULL) {
        return NULL;
    }

    tmp = list->prev;
    list->prev = NULL;

    ret = bodhi_list_new(tmp->data);
    while ((tmp = tmp->prev)) {
        bodhi_list_add(ret, tmp->data);
    }

    list->prev = tmp;
    return ret;
}

bodhi_list_t *bodhi_list_nth(bodhi_list_t *list, size_t n) {
    bodhi_list_t *ret = list;
    size_t i;

    for (i = n; i > 0; i--) {
        if (ret == NULL) {
            return NULL;
        }
        ret = ret->next;
    }

    return ret;
}

void *bodhi_list_find(const bodhi_list_t *list, const void *needle, bodhi_list_cmp_fn fn) {
    const bodhi_list_t *iter;

    for (iter = list; iter; iter = iter->next) {
        if (iter->data != NULL && fn(iter->data, needle) == 0) {
            return iter->data;
        }
    }

    return NULL;
}

bodhi_list_t *bodhi_list_last(bodhi_list_t *list) {
    if (list != NULL) {
        return list->prev;
    } else {
        return NULL;
    }
}

size_t bodhi_list_count(bodhi_list_t *list) {
    bodhi_list_t *iter;
    size_t s = 0;

    for (iter = list; iter; iter = iter->next) {
        s++;
    }

    return s;
}

void **bodhi_list_to_array(bodhi_list_t *list, size_t size) {
    size_t count = bodhi_list_count(list);
    bodhi_list_t *iter;
    size_t siter;
    void **ret;

    if (count == 0) {
        return NULL;
    }

    CALLOC(ret, count, size, return NULL);

    for (iter = list, siter = 0; siter < count; iter = iter->next, siter++) {
        *(ret + siter) = iter->data;
    }

    return ret;
}
