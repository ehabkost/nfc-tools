/*
 * $Id: nfcconf.h 233 2007-04-04 09:52:54Z ludovic.rousseau $
 *
 * Copyright (C) 2002
 *  Antti Tapaninen <aet@cc.hut.fi>
 *
 * Originally based on source by Timo Sirainen <tss@iki.fi>
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

#ifndef _SC_CONF_H
#define _SC_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _nfcconf_entry {
	const char *name;
	unsigned int type;
	unsigned int flags;
	void *parm;
	void *arg;
} nfcconf_entry;

/* Entry flags */
#define SCCONF_PRESENT		0x00000001
#define SCCONF_MANDATORY	0x00000002
#define SCCONF_ALLOC		0x00000004
#define SCCONF_ALL_BLOCKS	0x00000008
#define SCCONF_VERBOSE		0x00000010	/* For debugging purposes only */

/* Entry types */
#define SCCONF_CALLBACK		1
#define SCCONF_BLOCK		2
#define SCCONF_LIST		3

#define SCCONF_BOOLEAN		11
#define SCCONF_INTEGER		12
#define SCCONF_STRING		13

typedef struct _nfcconf_block nfcconf_block;

typedef struct _nfcconf_list {
	struct _nfcconf_list *next;
	char *data;
} nfcconf_list;

#define SCCONF_ITEM_TYPE_COMMENT	0	/* key = NULL, comment */
#define SCCONF_ITEM_TYPE_BLOCK		1	/* key = key, block */
#define SCCONF_ITEM_TYPE_VALUE		2	/* key = key, list */

typedef struct _nfcconf_item {
	struct _nfcconf_item *next;
	int type;
	char *key;
	union {
		char *comment;
		nfcconf_block *block;
		nfcconf_list *list;
	} value;
} nfcconf_item;

struct _nfcconf_block {
	nfcconf_block *parent;
	nfcconf_list *name;
	nfcconf_item *items;
};

typedef struct {
	char *filename;
	int debug;
	nfcconf_block *root;
	char *errmsg;
} nfcconf_context;

/* Allocate nfcconf_context
 * The filename can be NULL
 */
extern nfcconf_context *nfcconf_new(const char *filename);

/* Free nfcconf_context
 */
extern void nfcconf_free(nfcconf_context * config);

/* Parse configuration
 * Returns 1 = ok, 0 = error, -1 = error opening config file
 */
extern int nfcconf_parse(nfcconf_context * config);

/* Parse a static configuration string
 * Returns 1 = ok, 0 = error
 */
extern int nfcconf_parse_string(nfcconf_context * config, const char *string);

/* Parse entries
 */
extern int nfcconf_parse_entries(const nfcconf_context * config, const nfcconf_block * block, nfcconf_entry * entry);

/* Write config to a file
 * If the filename is NULL, use the config->filename
 * Returns 0 = ok, else = errno
 */
extern int nfcconf_write(nfcconf_context * config, const char *filename);

/* Write configuration entries to block
 */
extern int nfcconf_write_entries(nfcconf_context * config, nfcconf_block * block, nfcconf_entry * entry);

/* Find a block by the item_name
 * If the block is NULL, the root block is used
 */
extern const nfcconf_block *nfcconf_find_block(const nfcconf_context * config, const nfcconf_block * block, const char *item_name);

/* Find blocks by the item_name
 * If the block is NULL, the root block is used
 * The key can be used to specify what the blocks first name should be
 */
extern nfcconf_block **nfcconf_find_blocks(const nfcconf_context * config, const nfcconf_block * block, const char *item_name, const char *key);

/* Get a list of values for option
 */
extern const nfcconf_list *nfcconf_find_list(const nfcconf_block * block, const char *option);

/* Return the first string of the option
 * If no option found, return def
 */
extern const char *nfcconf_get_str(const nfcconf_block * block, const char *option, const char *def);

/* Return the first value of the option as integer
 * If no option found, return def
 */
extern int nfcconf_get_int(const nfcconf_block * block, const char *option, int def);

/* Return the first value of the option as boolean
 * If no option found, return def
 */
extern int nfcconf_get_bool(const nfcconf_block * block, const char *option, int def);

/* Write value to a block as a string
 */
extern const char *nfcconf_put_str(nfcconf_block * block, const char *option, const char *value);

/* Write value to a block as an integer
 */
extern int nfcconf_put_int(nfcconf_block * block, const char *option, int value);

/* Write value to a block as a boolean
 */
extern int nfcconf_put_bool(nfcconf_block * block, const char *option, int value);

/* Add block structure
 * If the block is NULL, the root block is used
 */
extern nfcconf_block *nfcconf_block_add(nfcconf_context * config, nfcconf_block * block, const char *key, const nfcconf_list *name);

/* Copy block structure (recursive)
 */
extern nfcconf_block *nfcconf_block_copy(const nfcconf_block * src, nfcconf_block ** dst);

/* Free block structure (recursive)
 */
extern void nfcconf_block_destroy(nfcconf_block * block);

/* Add item to block structure
 * If the block is NULL, the root block is used
 */
extern nfcconf_item *nfcconf_item_add(nfcconf_context * config, nfcconf_block * block, nfcconf_item * item, int type, const char *key, const void *data);

/* Copy item structure (recursive)
 */
extern nfcconf_item *nfcconf_item_copy(const nfcconf_item * src, nfcconf_item ** dst);

/* Free item structure (recursive)
 */
extern void nfcconf_item_destroy(nfcconf_item * item);

/* Add a new value to the list
 */
extern nfcconf_list *nfcconf_list_add(nfcconf_list ** list, const char *value);

/* Copy list structure
 */
extern nfcconf_list *nfcconf_list_copy(const nfcconf_list * src, nfcconf_list ** dst);

/* Free list structure
 */
extern void nfcconf_list_destroy(nfcconf_list * list);

/* Return the length of an list array
 */
extern int nfcconf_list_array_length(const nfcconf_list * list);

/* Return the combined length of the strings on all arrays
 */
extern int nfcconf_list_strings_length(const nfcconf_list * list);

/* Return an allocated string that contains all
 * the strings in a list separated by the filler
 * The filler can be NULL
 */
extern char *nfcconf_list_strdup(const nfcconf_list * list, const char *filler);

/* Returns an allocated array of const char *pointers to
 * list elements.
 * Last pointer is NULL
 * Array must be freed, but pointers to strings belong to nfcconf_list
 */
extern const char **nfcconf_list_toarray(const nfcconf_list * list);

#ifdef __cplusplus
}
#endif
#endif
