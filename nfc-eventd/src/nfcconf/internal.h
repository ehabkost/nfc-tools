/*
 * $Id: internal.h 233 2007-04-04 09:52:54Z ludovic.rousseau $
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

#ifndef _SCCONF_INTERNAL_H
#define _SCCONF_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#define TOKEN_TYPE_COMMENT	0
#define TOKEN_TYPE_NEWLINE	1
#define TOKEN_TYPE_STRING	2
#define TOKEN_TYPE_PUNCT	3

    typedef struct _nfcconf_parser {
        nfcconf_context *config;

        nfcconf_block *block;
        nfcconf_item *last_item, *current_item;

        char *key;
        nfcconf_list *name;

        int state;
        int last_token_type;
        int line;

unsigned int error:
        1;
unsigned int warnings:
        1;
        char emesg[256];
    } nfcconf_parser;

    extern int nfcconf_lex_parse(nfcconf_parser * parser, const char *filename);
    extern int nfcconf_lex_parse_string(nfcconf_parser * parser,
                                            const char *config_string);
    extern void nfcconf_parse_token(nfcconf_parser * parser, int token_type, const char *token);

#ifdef __cplusplus
}
#endif
#endif
