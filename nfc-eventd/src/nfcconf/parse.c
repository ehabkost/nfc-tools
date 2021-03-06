/*
 * $Id: parse.c 233 2007-04-04 09:52:54Z ludovic.rousseau $
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
  #include "config.h"
#endif // HAVE_CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#include <errno.h>
#include "nfcconf.h"
#include "internal.h"

#define STATE_NAME	0x01
#define STATE_VALUE	0x02
#define STATE_SET	0x10

static nfcconf_item *nfcconf_get_last_item(nfcconf_block *root) {
    nfcconf_block *block = root;
    nfcconf_item *item;

    for (item = root->items; item; item = item->next) {
        if (!item->next) {
            return item;
        }
    }
    return block->items;
}

static void nfcconf_parse_error(nfcconf_parser * parser, const char *error) {
    /* FIXME: save the error somewhere */
    parser->error = 1;

    snprintf(parser->emesg, sizeof(parser->emesg), "Line %d: %s\n", parser->line, error);
}

static void nfcconf_parse_error_not_expect(nfcconf_parser * parser,
        const char *token) {
    /* FIXME: save the error somewhere */
    parser->error = 1;

    snprintf(parser->emesg, sizeof(parser->emesg), "Line %d: not expecting '%s'\n", parser->line, token);
}

static void nfcconf_parse_warning_expect(nfcconf_parser * parser, const char *token) {
    /* FIXME: save the warnings somewhere */
    parser->warnings = 1;

    snprintf(parser->emesg, sizeof(parser->emesg),
             "Line %d: missing '%s', ignoring\n",
             parser->line, token);
}

static nfcconf_item *nfcconf_item_find(nfcconf_parser * parser, const char *key) {
    nfcconf_item *item;

    for (item = parser->block->items; item; item = item->next) {
        if (item->type == SCCONF_ITEM_TYPE_VALUE &&
                strcasecmp(item->key, parser->key) == 0) {
            return item;
        }
    }
    return item;
}

static nfcconf_item *nfcconf_item_add_internal(nfcconf_parser * parser, int type) {
    nfcconf_item *item;

    if (type == SCCONF_ITEM_TYPE_VALUE) {
        /* if item with same key already exists, use it */
        item = nfcconf_item_find(parser, parser->key);
        if (item) {
            if (parser->key) {
                free(parser->key);
            }
            parser->key = NULL;
            parser->current_item = item;
            return item;
        }
    }
    item = (nfcconf_item *) malloc(sizeof(nfcconf_item));
    if (!item) {
        return NULL;
    }
    memset(item, 0, sizeof(nfcconf_item));
    item->type = type;

    item->key = parser->key;
    parser->key = NULL;

    if (parser->last_item) {
        parser->last_item->next = item;
    } else {
        parser->block->items = item;
    }
    parser->current_item = parser->last_item = item;
    return item;
}

nfcconf_item *nfcconf_item_add(nfcconf_context * config, nfcconf_block * block, nfcconf_item * item, int type, const char *key, const void *data) {
    nfcconf_parser parser;
    nfcconf_block *dst = NULL;

    if (!config && !block)
        return NULL;
    if (!data)
        return NULL;

    memset(&parser, 0, sizeof(nfcconf_parser));
    parser.config = config ? config : NULL;
    parser.key = key ? strdup(key) : NULL;
    parser.block = block ? block : config->root;
    parser.name = NULL;
    parser.last_item = nfcconf_get_last_item(parser.block);
    parser.current_item = item;

    if (type == SCCONF_ITEM_TYPE_BLOCK) {
        nfcconf_block_copy((const nfcconf_block *) data, &dst);
        nfcconf_list_copy(dst->name, &parser.name);
    }
    nfcconf_item_add_internal(&parser, type);
    switch (parser.current_item->type) {
    case SCCONF_ITEM_TYPE_COMMENT:
        parser.current_item->value.comment = strdup((char *) data);
        break;
    case SCCONF_ITEM_TYPE_BLOCK:
        dst->parent = parser.block;
        parser.current_item->value.block = dst;
        nfcconf_list_destroy(parser.name);
        break;
    case SCCONF_ITEM_TYPE_VALUE:
        nfcconf_list_copy((const nfcconf_list *) data, &parser.current_item->value.list);
        break;
    }
    return parser.current_item;
}

static void nfcconf_block_add_internal(nfcconf_parser * parser) {
    nfcconf_block *block;
    nfcconf_item *item;

    item = nfcconf_item_add_internal(parser, SCCONF_ITEM_TYPE_BLOCK);

    block = (nfcconf_block *) malloc(sizeof(nfcconf_block));
    if (!block) {
        return;
    }
    memset(block, 0, sizeof(nfcconf_block));
    block->parent = parser->block;
    item->value.block = block;

    if (!parser->name) {
        nfcconf_list_add(&parser->name, "");
    }
    block->name = parser->name;
    parser->name = NULL;

    parser->block = block;
    parser->last_item = NULL;
}

nfcconf_block *nfcconf_block_add(nfcconf_context * config, nfcconf_block * block, const char *key, const nfcconf_list *name) {
    nfcconf_parser parser;

    memset(&parser, 0, sizeof(nfcconf_parser));
    parser.config = config ? config : NULL;
    parser.key = key ? strdup(key) : NULL;
    parser.block = block ? block : config->root;
    nfcconf_list_copy(name, &parser.name);
    parser.last_item = nfcconf_get_last_item(parser.block);
    parser.current_item = parser.block->items;

    nfcconf_block_add_internal(&parser);
    return parser.block;
}

static void nfcconf_parse_parent(nfcconf_parser * parser) {
    parser->block = parser->block->parent;

    parser->last_item = parser->block->items;
    if (parser->last_item) {
        while (parser->last_item->next) {
            parser->last_item = parser->last_item->next;
        }
    }
}

static void nfcconf_parse_reset_state(nfcconf_parser * parser) {
    if (parser) {
        if (parser->key) {
            free(parser->key);
        }
        nfcconf_list_destroy(parser->name);

        parser->key = NULL;
        parser->name = NULL;
        parser->state = 0;
    }
}

void nfcconf_parse_token(nfcconf_parser * parser, int token_type, const char *token) {
    nfcconf_item *item;
    int len;

    if (parser->error) {
        /* fatal error */
        return;
    }
    switch (token_type) {
    case TOKEN_TYPE_NEWLINE:
        parser->line++;
        if (parser->last_token_type != TOKEN_TYPE_NEWLINE) {
            break;
        }
        /* fall through - treat empty lines as comments */
    case TOKEN_TYPE_COMMENT:
        item = nfcconf_item_add_internal(parser, SCCONF_ITEM_TYPE_COMMENT);
        item->value.comment = token ? strdup(token) : NULL;
        break;
    case TOKEN_TYPE_STRING: {
        char *stoken = NULL;

        if ((parser->state & (STATE_VALUE | STATE_SET)) ==
                (STATE_VALUE | STATE_SET)) {
            nfcconf_parse_warning_expect(parser, ";");
            nfcconf_parse_reset_state(parser);
        }
        if (*token == '"') {
            /* quoted string, remove them */
            token++;
            len = strlen(token);
            if (len < 1 || token[len - 1] != '"') {
                nfcconf_parse_warning_expect(parser, "\"");
            } else {
                /* stoken */
                stoken = token ? strdup(token) : NULL;
                if (stoken) {
                    stoken[len - 1] = '\0';
                }
            }
        }
        if (!stoken) {
            stoken = token ? strdup(token) : NULL;
        }
        if (parser->state == 0) {
            /* key */
            parser->key = stoken ? strdup(stoken) : NULL;
            parser->state = STATE_NAME;
        } else if (parser->state == STATE_NAME) {
            /* name */
            parser->state |= STATE_SET;
            nfcconf_list_add(&parser->name, stoken);
        } else if (parser->state == STATE_VALUE) {
            /* value */
            parser->state |= STATE_SET;
            nfcconf_list_add(&parser->current_item->value.list,
                             stoken);
        } else {
            /* error */
            nfcconf_parse_error_not_expect(parser, stoken);
        }
        if (stoken) {
            free(stoken);
        }
        stoken = NULL;
    }
    break;
    case TOKEN_TYPE_PUNCT:
        switch (*token) {
        case '{':
            if ((parser->state & STATE_NAME) == 0) {
                nfcconf_parse_error_not_expect(parser, "{");
                break;
            }
            nfcconf_block_add_internal(parser);
            nfcconf_parse_reset_state(parser);
            break;
        case '}':
            if (parser->state != 0) {
                if ((parser->state & STATE_VALUE) == 0 ||
                        (parser->state & STATE_SET) == 0) {
                    nfcconf_parse_error_not_expect(parser,
                                                   "}");
                    break;
                }
                /* foo = bar } */
                nfcconf_parse_warning_expect(parser, ";");
                nfcconf_parse_reset_state(parser);
            }
            if (!parser->block->parent) {
                /* too many '}' */
                nfcconf_parse_error(parser,
                                    "missing matching '{'");
                break;
            }
            nfcconf_parse_parent(parser);
            break;
        case ',':
            if ((parser->state & (STATE_NAME | STATE_VALUE)) == 0) {
                nfcconf_parse_error_not_expect(parser, ",");
            }
            parser->state &= ~STATE_SET;
            break;
        case '=':
            if ((parser->state & STATE_NAME) == 0) {
                nfcconf_parse_error_not_expect(parser, "=");
                break;
            }
            nfcconf_item_add_internal(parser, SCCONF_ITEM_TYPE_VALUE);
            parser->state = STATE_VALUE;
            break;
        case ';':
#if 0
            if ((parser->state & STATE_VALUE) == 0 ||
                    (parser->state & STATE_SET) == 0) {
                nfcconf_parse_error_not_expect(parser, ";");
                break;
            }
#endif
            nfcconf_parse_reset_state(parser);
            break;
        default:
            snprintf(parser->emesg, sizeof(parser->emesg),
                     "Line %d: bad token ignoring\n",
                     parser->line);
        }
        break;
    }

    parser->last_token_type = token_type;
}

int nfcconf_parse(nfcconf_context * config) {
    static char buffer[256];
    nfcconf_parser p;
    int r = 1;

    memset(&p, 0, sizeof(p));
    p.config = config;
    p.block = config->root;
    p.line = 1;

    if (!nfcconf_lex_parse(&p, config->filename)) {
        snprintf(buffer, sizeof(buffer),
                 "Unable to open \"%s\": %s",
                 config->filename, strerror(errno));
        r = -1;
    } else if (p.error) {
        strncpy(buffer, p.emesg, sizeof(buffer)-1);
        r = 0;
    } else {
        r = 1;
    }

    if (r <= 0)
        config->errmsg = buffer;
    return r;
}

int nfcconf_parse_string(nfcconf_context * config, const char *string) {
    static char buffer[256];
    nfcconf_parser p;
    int r;

    memset(&p, 0, sizeof(p));
    p.config = config;
    p.block = config->root;
    p.line = 1;

    if (!nfcconf_lex_parse_string(&p, string)) {
        snprintf(buffer, sizeof(buffer),
                 "Failed to parse configuration string");
        r = -1;
    } else if (p.error) {
        strncpy(buffer, p.emesg, sizeof(buffer)-1);
        r = 0;
    } else {
        r = 1;
    }

    if (r <= 0)
        config->errmsg = buffer;
    return r;
}
