/*
 * $Id: nfcconf.c 233 2007-04-04 09:52:54Z ludovic.rousseau $
 *
 * Copyright (C) 2002
 *  Antti Tapaninen <aet@cc.hut.fi>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _SVID_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include <ctype.h>
#include "nfcconf.h"

nfcconf_context *nfcconf_new(const char *filename) {
    nfcconf_context *config;

    config = (nfcconf_context *) malloc(sizeof(nfcconf_context));
    if (!config) {
        return NULL;
    }
    memset(config, 0, sizeof(nfcconf_context));
    config->filename = filename ? strdup(filename) : NULL;
    config->root = (nfcconf_block *) malloc(sizeof(nfcconf_block));
    if (!config->root) {
        if (config->filename) {
            free(config->filename);
        }
        free(config);
        return NULL;
    }
    memset(config->root, 0, sizeof(nfcconf_block));
    return config;
}

void nfcconf_free(nfcconf_context * config) {
    if (config) {
        nfcconf_block_destroy(config->root);
        if (config->filename) {
            free(config->filename);
        }
        free(config);
    }
}

const nfcconf_block *nfcconf_find_block(const nfcconf_context * config, const nfcconf_block * block, const char *item_name) {
    nfcconf_item *item;

    if (!block) {
        block = config->root;
    }
    if (!item_name) {
        return NULL;
    }
    for (item = block->items; item; item = item->next) {
        if (item->type == SCCONF_ITEM_TYPE_BLOCK &&
                strcasecmp(item_name, item->key) == 0) {
            return item->value.block;
        }
    }
    return NULL;
}

nfcconf_block **nfcconf_find_blocks(const nfcconf_context * config, const nfcconf_block * block, const char *item_name, const char *key) {
    nfcconf_block **blocks = NULL, **tmp;
    int alloc_size, size;
    nfcconf_item *item;

    if (!block) {
        block = config->root;
    }
    if (!item_name) {
        return NULL;
    }
    size = 0;
    alloc_size = 10;
    blocks = (nfcconf_block **) realloc(blocks, sizeof(nfcconf_block *) * alloc_size);

    for (item = block->items; item; item = item->next) {
        if (item->type == SCCONF_ITEM_TYPE_BLOCK &&
                strcasecmp(item_name, item->key) == 0) {
            if (key && strcasecmp(key, item->value.block->name->data)) {
                continue;
            }
            if (size + 1 >= alloc_size) {
                alloc_size *= 2;
                tmp = (nfcconf_block **) realloc(blocks, sizeof(nfcconf_block *) * alloc_size);
                if (!tmp) {
                    free(blocks);
                    return NULL;
                }
                blocks = tmp;
            }
            blocks[size++] = item->value.block;
        }
    }
    blocks[size] = NULL;
    return blocks;
}

const nfcconf_list *nfcconf_find_list(const nfcconf_block * block, const char *option) {
    nfcconf_item *item;

    if (!block) {
        return NULL;
    }
    for (item = block->items; item; item = item->next) {
        if (item->type == SCCONF_ITEM_TYPE_VALUE &&
                strcasecmp(option, item->key) == 0) {
            return item->value.list;
        }
    }
    return NULL;
}

const char *nfcconf_get_str(const nfcconf_block * block, const char *option, const char *def) {
    const nfcconf_list *list;

    list = nfcconf_find_list(block, option);
    return !list ? def : list->data;
}

int nfcconf_get_int(const nfcconf_block * block, const char *option, int def) {
    const nfcconf_list *list;

    list = nfcconf_find_list(block, option);
    return !list ? def : strtol(list->data, NULL, 0);
}

int nfcconf_get_bool(const nfcconf_block * block, const char *option, int def) {
    const nfcconf_list *list;

    list = nfcconf_find_list(block, option);
    if (!list) {
        return def;
    }
    return toupper((int) *list->data) == 'T' || toupper((int) *list->data) == 'Y';
}

const char *nfcconf_put_str(nfcconf_block * block, const char *option, const char *value) {
    nfcconf_list *list = NULL;
    nfcconf_item *item;

    nfcconf_list_add(&list, value);
    item = nfcconf_item_add(NULL, block, NULL, SCCONF_ITEM_TYPE_VALUE, option, list);
    nfcconf_list_destroy(list);
    return value;
}

int nfcconf_put_int(nfcconf_block * block, const char *option, int value) {
    const char *ret;
    char *str;

    str = (char *) malloc(64);
    if (!str) {
        return value;
    }
    snprintf(str, 64, "%i", value);
    ret = nfcconf_put_str(block, option, str);
    free(str);
    return value;
}

int nfcconf_put_bool(nfcconf_block * block, const char *option, int value) {
    const char *ret;

    ret = nfcconf_put_str(block, option, !value ? "false" : "true");
    return value;
}

nfcconf_item *nfcconf_item_copy(const nfcconf_item * src, nfcconf_item ** dst) {
    nfcconf_item *ptr, *_dst = NULL, *next = NULL;

    next = (nfcconf_item *) malloc(sizeof(nfcconf_item));
    if (!next) {
        return NULL;
    }
    memset(next, 0, sizeof(nfcconf_item));
    ptr = next;
    _dst = next;
    while (src) {
        if (!next) {
            next = (nfcconf_item *) malloc(sizeof(nfcconf_item));
            if (!next) {
                nfcconf_item_destroy(ptr);
                return NULL;
            }
            memset(next, 0, sizeof(nfcconf_item));
            _dst->next = next;
        }
        next->type = src->type;
        switch (src->type) {
        case SCCONF_ITEM_TYPE_COMMENT:
            next->value.comment = src->value.comment ? strdup(src->value.comment) : NULL;
            break;
        case SCCONF_ITEM_TYPE_BLOCK:
            nfcconf_block_copy(src->value.block, &next->value.block);
            break;
        case SCCONF_ITEM_TYPE_VALUE:
            nfcconf_list_copy(src->value.list, &next->value.list);
            break;
        }
        next->key = src->key ? strdup(src->key) : NULL;
        _dst = next;
        next = NULL;
        src = src->next;
    }
    *dst = ptr;
    return ptr;
}

void nfcconf_item_destroy(nfcconf_item * item) {
    nfcconf_item *next;

    while (item) {
        next = item->next;

        switch (item->type) {
        case SCCONF_ITEM_TYPE_COMMENT:
            if (item->value.comment) {
                free(item->value.comment);
            }
            item->value.comment = NULL;
            break;
        case SCCONF_ITEM_TYPE_BLOCK:
            nfcconf_block_destroy(item->value.block);
            break;
        case SCCONF_ITEM_TYPE_VALUE:
            nfcconf_list_destroy(item->value.list);
            break;
        }

        if (item->key) {
            free(item->key);
        }
        item->key = NULL;
        free(item);
        item = next;
    }
}

nfcconf_block *nfcconf_block_copy(const nfcconf_block * src, nfcconf_block ** dst) {
    if (src) {
        nfcconf_block *_dst = NULL;

        _dst = (nfcconf_block *) malloc(sizeof(nfcconf_block));
        if (!_dst) {
            return NULL;
        }
        memset(_dst, 0, sizeof(nfcconf_block));
        if (src->name) {
            nfcconf_list_copy(src->name, &_dst->name);
        }
        if (src->items) {
            nfcconf_item_copy(src->items, &_dst->items);
        }
        *dst = _dst;
        return _dst;
    }
    return NULL;
}

void nfcconf_block_destroy(nfcconf_block * block) {
    if (block) {
        nfcconf_list_destroy(block->name);
        nfcconf_item_destroy(block->items);
        free(block);
    }
}

nfcconf_list *nfcconf_list_add(nfcconf_list ** list, const char *value) {
    nfcconf_list *rec, **tmp;

    rec = (nfcconf_list *) malloc(sizeof(nfcconf_list));
    if (!rec) {
        return NULL;
    }
    memset(rec, 0, sizeof(nfcconf_list));
    rec->data = value ? strdup(value) : NULL;

    if (!*list) {
        *list = rec;
    } else {
        for (tmp = list; *tmp; tmp = &(*tmp)->next);
        *tmp = rec;
    }
    return rec;
}

nfcconf_list *nfcconf_list_copy(const nfcconf_list * src, nfcconf_list ** dst) {
    nfcconf_list *next;

    while (src) {
        next = src->next;
        nfcconf_list_add(dst, src->data);
        src = next;
    }
    return *dst;
}

void nfcconf_list_destroy(nfcconf_list * list) {
    nfcconf_list *next;

    while (list) {
        next = list->next;
        if (list->data) {
            free(list->data);
        }
        free(list);
        list = next;
    }
}

int nfcconf_list_array_length(const nfcconf_list * list) {
    int len = 0;

    while (list) {
        len++;
        list = list->next;
    }
    return len;
}

int nfcconf_list_strings_length(const nfcconf_list * list) {
    int len = 0;

    while (list && list->data) {
        len += strlen(list->data) + 1;
        list = list->next;
    }
    return len;
}

const char **nfcconf_list_toarray(const nfcconf_list * list) {
    const nfcconf_list * lp = list;
    const char **tp;
    int len = 0;

    while (lp) {
        len++;
        lp = lp->next;
    }
    tp = (const char **)malloc(sizeof(char *) * (len + 1));
    if (!tp)
        return tp;
    lp = list;
    len = 0;
    while (lp) {
        tp[len] = lp->data;
        len++;
        lp = lp->next;
    }
    tp[len] = NULL;
    return tp;
}

char *nfcconf_list_strdup(const nfcconf_list * list, const char *filler) {
    char *buf = NULL;
    int len = 0;

    if (!list) {
        return NULL;
    }
    len = nfcconf_list_strings_length(list);
    if (filler) {
        len += nfcconf_list_array_length(list) * (strlen(filler) + 1);
    }
    buf = (char *) malloc(len);
    if (!buf) {
        return NULL;
    }
    memset(buf, 0, len);
    while (list && list->data) {
        strcat(buf, list->data);
        if (filler) {
            strcat(buf, filler);
        }
        list = list->next;
    }
    if (filler)
        buf[strlen(buf) - strlen(filler)] = '\0';
    return buf;
}

static nfcconf_block **getblocks(const nfcconf_context * config, const nfcconf_block * block, nfcconf_entry * entry) {
    nfcconf_block **blocks = NULL, **tmp;

    blocks = nfcconf_find_blocks(config, block, entry->name, NULL);
    if (blocks) {
        if (blocks[0] != NULL) {
            if (config->debug) {
                fprintf(stderr, "block found (%s)\n", entry->name);
            }
            return blocks;
        }
        free(blocks);
        blocks = NULL;
    }
    if (nfcconf_find_list(block, entry->name) != NULL) {
        if (config->debug) {
            fprintf(stderr, "list found (%s)\n", entry->name);
        }
        tmp = (nfcconf_block **) realloc(blocks, sizeof(nfcconf_block *) * 2);
        if (!tmp) {
            free(blocks);
            return NULL;
        }
        blocks = tmp;
        blocks[0] = (nfcconf_block *) block;
        blocks[1] = NULL;
    }
    return blocks;
}

static int parse_entries(const nfcconf_context * config, const nfcconf_block * block, nfcconf_entry * entry, int depth);

static int parse_type(const nfcconf_context * config, const nfcconf_block * block, nfcconf_entry * entry, int depth) {
    void *parm = entry->parm;
    size_t *len = (size_t *) entry->arg;
    int (*callback_func) (const nfcconf_context * config, const nfcconf_block * block, nfcconf_entry * entry, int depth) =
        (int (*)(const nfcconf_context *, const nfcconf_block *, nfcconf_entry *, int)) parm;
    int r = 0;

    if (config->debug) {
        fprintf(stderr, "decoding '%s'\n", entry->name);
    }
    switch (entry->type) {
    case SCCONF_CALLBACK:
        if (parm) {
            r = callback_func(config, block, entry, depth);
        }
        break;
    case SCCONF_BLOCK:
        if (parm) {
            r = parse_entries(config, block, (nfcconf_entry *) parm, depth + 1);
        }
        break;
    case SCCONF_LIST: {
        const nfcconf_list *val = nfcconf_find_list(block, entry->name);

        if (!val) {
            r = 1;
            break;
        }
        if (parm) {
            if (entry->flags & SCCONF_ALLOC) {
                nfcconf_list *dest = NULL;

                for (; val != NULL; val = val->next) {
                    if (!nfcconf_list_add(&dest, val->data)) {
                        r = 1;
                        break;
                    }
                }
                *((nfcconf_list **) parm) = dest;
            } else {
                *((const nfcconf_list **) parm) = val;
            }
        }
        if (entry->flags & SCCONF_VERBOSE) {
            char *buf = nfcconf_list_strdup(val, ", ");
            printf("%s = %s\n", entry->name, buf);
            free(buf);
        }
    }
    break;
    case SCCONF_BOOLEAN: {
        int val = nfcconf_get_bool(block, entry->name, 0);

        if (parm) {
            *((int *) parm) = val;
        }
        if (entry->flags & SCCONF_VERBOSE) {
            printf("%s = %s\n", entry->name, val == 0 ? "false" : "true");
        }
    }
    break;
    case SCCONF_INTEGER: {
        int val = nfcconf_get_int(block, entry->name, 0);

        if (parm) {
            *((int *) parm) = val;
        }
        if (entry->flags & SCCONF_VERBOSE) {
            printf("%s = %i\n", entry->name, val);
        }
    }
    break;
    case SCCONF_STRING: {
        const char *val = nfcconf_get_str(block, entry->name, NULL);
        int vallen = val ? strlen(val) : 0;

        if (!vallen) {
            r = 1;
            break;
        }
        if (parm) {
            if (entry->flags & SCCONF_ALLOC) {
                char **buf = (char **) parm;
                *buf = (char *) malloc(vallen + 1);
                if (*buf == NULL) {
                    r = 1;
                    break;
                }
                memset(*buf, 0, vallen + 1);
                if (len) {
                    *len = vallen;
                }
                parm = *buf;
            }
            memcpy((char *) parm, val, vallen);
        }
        if (entry->flags & SCCONF_VERBOSE) {
            printf("%s = %s\n", entry->name, val);
        }
    }
    break;
    default:
        fprintf(stderr, "invalid configuration type: %d\n", entry->type);
    }
    if (r) {
        fprintf(stderr, "decoding of configuration entry '%s' failed.\n", entry->name);
        return r;
    }
    entry->flags |= SCCONF_PRESENT;
    return 0;
}

static int parse_entries(const nfcconf_context * config, const nfcconf_block * block, nfcconf_entry * entry, int depth) {
    int r, i, idx;
    nfcconf_entry *e;
    nfcconf_block **blocks = NULL;

    if (config->debug) {
        fprintf(stderr, "parse_entries called, depth %d\n", depth);
    }
    for (idx = 0; entry[idx].name; idx++) {
        e = &entry[idx];
        r = 0;
        blocks = getblocks(config, block, e);
        if (!blocks) {
            if (!(e->flags & SCCONF_MANDATORY)) {
                if (config->debug)
                    fprintf(stderr, "optional configuration entry '%s' not present\n",
                            e->name);
                continue;
            }
            fprintf(stderr, "mandatory configuration entry '%s' not found\n", e->name);
            return 1;
        }
        for (i = 0; blocks[i]; i++) {
            r = parse_type(config, blocks[i], e, depth);
            if (r) {
                free(blocks);
                return r;
            }
            if (!(e->flags & SCCONF_ALL_BLOCKS))
                break;
        }
        free(blocks);
    }
    return 0;
}

int nfcconf_parse_entries(const nfcconf_context * config, const nfcconf_block * block, nfcconf_entry * entry) {
    if (!entry)
        return 1;
    if (!block)
        block = config->root;
    return parse_entries(config, block, entry, 0);
}

static int write_entries(nfcconf_context * config, nfcconf_block * block, nfcconf_entry * entry, int depth);

static int write_type(nfcconf_context * config, nfcconf_block * block, nfcconf_entry * entry, int depth) {
    void *parm = entry->parm;
    void *arg = entry->arg;
    int (*callback_func) (nfcconf_context * config, nfcconf_block * block, nfcconf_entry * entry, int depth) =
        (int (*)(nfcconf_context *, nfcconf_block *, nfcconf_entry *, int)) parm;
    int r = 0;

    if (config->debug) {
        fprintf(stderr, "encoding '%s'\n", entry->name);
    }
    switch (entry->type) {
    case SCCONF_CALLBACK:
        if (parm) {
            r = callback_func(config, block, entry, depth);
        }
        break;
    case SCCONF_BLOCK:
        if (parm) {
            nfcconf_block *subblock;
            const nfcconf_list *name = (const nfcconf_list *) arg;

            subblock = nfcconf_block_add(config, block, entry->name, name);
            r = write_entries(config, subblock, (nfcconf_entry *) parm, depth + 1);
        }
        break;
    case SCCONF_LIST:
        if (parm) {
            const nfcconf_list *val = (const nfcconf_list *) parm;

            nfcconf_item_add(config, block, NULL, SCCONF_ITEM_TYPE_VALUE, entry->name, val);
            if (entry->flags & SCCONF_VERBOSE) {
                char *buf = nfcconf_list_strdup(val, ", ");
                printf("%s = %s\n", entry->name, buf);
                free(buf);
            }
        }
        break;
    case SCCONF_BOOLEAN:
        if (parm) {
            const int val = parm ? (int) parm : 0;

            nfcconf_put_bool(block, entry->name, val);
            if (entry->flags & SCCONF_VERBOSE) {
                printf("%s = %s\n", entry->name, val == 0 ? "false" : "true");
            }
        }
        break;
    case SCCONF_INTEGER:
        if (parm) {
            const int val = parm ? (int) parm : 0;

            nfcconf_put_int(block, entry->name, val);
            if (entry->flags & SCCONF_VERBOSE) {
                printf("%s = %i\n", entry->name, val);
            }
        }
        break;
    case SCCONF_STRING:
        if (parm) {
            const char *val = parm ? (const char *) parm : "";

            nfcconf_put_str(block, entry->name, val);
            if (entry->flags & SCCONF_VERBOSE) {
                printf("%s = %s\n", entry->name, val);
            }
        }
        break;
    default:
        fprintf(stderr, "invalid configuration type: %d\n", entry->type);
    }
    if (r) {
        fprintf(stderr, "encoding of configuration entry '%s' failed.\n", entry->name);
        return r;
    }
    entry->flags |= SCCONF_PRESENT;
    return 0;
}

static int write_entries(nfcconf_context * config, nfcconf_block * block, nfcconf_entry * entry, int depth) {
    int r, idx;
    nfcconf_entry *e;

    if (config->debug) {
        fprintf(stderr, "write_entries called, depth %d\n", depth);
    }
    for (idx = 0; entry[idx].name; idx++) {
        e = &entry[idx];
        r = 0;
        r = write_type(config, block, e, depth);
        if (r) {
            return r;
        }
    }
    return 0;
}

int nfcconf_write_entries(nfcconf_context * config, nfcconf_block * block, nfcconf_entry * entry) {
    if (!entry)
        return 1;
    if (!block)
        block = config->root;
    return write_entries(config, block, entry, 0);
}
