/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 *
 */
/* $Id: symbol.c,v 1.4 2002/05/28 16:26:16 robbiew Exp $ */
/*
 *			Generic Symbol Table
 *
 * This is intended to look a lot like ndbm, except that all information
 * is kept in memory, and a multi-key, hierarchical access mode is provided.
 * This is largely patterned after the Berkeley "DB" package.
 *
 *			    Requirements
 *
 *	- "key" is ASCII (a C string, in fact)
 *
 *			Symbol Table Structure
 *
 *	Two data structures:
 *		Symbol Table Header
 *		Symbol Table Node
 *
 *	A symbol table header consists of a magic number, a pointer to
 *	the first node in the symbol table, and a cursor that is used
 *	when sequentialy stepping thru the entire list.
 *
 *	Symbol table nodes contain a pointer to a key, a pointer to this
 *	key's data, and a pointer to the next node in the chain.
 *	Note that to create a hierarchical symbol table, a node is created
 *	whose data points to a symbol table header.
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "symbol.h"
#include "splitstr.h"

#define SYM_MAGIC	0xbadc0de

/*
 * Some functions can report an error message by assigning it to this
 * string.
 */

static char *sym_error = NULL;

/*
 *	Memory Allocators
 *
 * newsym() allocates a new symbol table header node
 * mknode(...) allocates a new symbol table entry
 */

SYM newsym(void)
{
	SYM h;

	if ((h = malloc(sizeof(struct symh))) == NULL) {
		sym_error = "sym header malloc failed!";
		return (NULL);
	}

	h->magic = SYM_MAGIC;
	h->sym = NULL;
	h->cursor = NULL;
	return (h);
}

static struct sym *mknode(struct sym *next, char *key, void *data)
{
	struct sym *n;

	if ((n = malloc(sizeof(struct sym))) == NULL) {
		sym_error = "sym node malloc failed!";
		return (NULL);
	}

	n->next = next;
	n->key = strdup(key);
	n->data = data;

	if (n->key == NULL) {
		sym_error = "sym node strdup(key) failed!";
		free(n);
		return (NULL);
	}
	return (n);
}

/*
 * Search for a key in a single-level symbol table hierarchy.
 */
static struct sym *find_key1(struct sym *sym, char *key)
{
	while (sym != NULL)
		if (strcmp(sym->key, key) == 0)
			return (sym);
		else
			sym = sym->next;
	return (NULL);
}

/*
 * Create a new key node, add it to the *end* of this list
 */
static int add_key(SYM sym, char *key, void *data)
{
	register struct sym *sn;

	if (sym->sym == NULL) {
		sym->sym = mknode(NULL, key, data);
		if (sym->sym == NULL) {
			return (-1);
		}
	} else {
		for (sn = sym->sym; sn != NULL && sn->next != NULL;
		     sn = sn->next) ;
		sn->next = mknode(NULL, key, data);
		assert(sn->next != NULL);
		if (sn->next == NULL)
			return (-1);
	}
	return (0);
}

/*
 *  Create a new symbol table
 */
SYM sym_open(int flags, int mode, int openinfo)
{
	return (newsym());
}

/*
 *	Add (key, data) to an existing symbol table
 *
 *  If the key does not exist, a new key is added to the end of the list.
 *  If the key exists and the PUT_REPLACE flag is not supplied, return EEXIST.
 *  If a symbol table entry in a multi-part key is not a symbol table (i.e.
 *  element two of a three or more element key), return ENOTDIR.
 *
 *  "data" is not duplicated and must not point to a static area that could
 *  go away before the element is deleted (such as a local string in a
 *  function)
 *
 *  "key" is duplicated as needed, and is not modified.
 *
 * Code:
 * chop up key on commas
 *
 * search until a key element isn't found in the key tree, the key list is
 * exhausted, or a key's data element is not a sub-tree
 *
 * if the key list is exhausted, return a "duplicate entry" error
 *
 * if the last found key's data element is not a sub-tree, return
 * something like "ENOTDIR".
 *
 * add new keys for sub-trees until key list is exhausted;
 * last node gets 'data'.
 *
 */
int sym_put(SYM sym, char *key, void *data, int flags)
{
	const char **keys;	/* key split into a 2d string array */
	char **kk;
	char *nkey;		/* copy of 'key' -- before split */
	SYM csym, ncsym;	/* search: current symbol table */
	struct sym *nsym = NULL;	/* search: found symbol entry */

	if (sym == NULL)
		return (EINVAL);

	nkey = strdup(key);
	keys = splitstr(key, ",", NULL);

	if (keys == NULL) {
		free(nkey);
		return (EINVAL);
	}

	for (kk = (char **)keys, csym = sym;
	     *kk != NULL && (nsym = find_key1(csym->sym, *kk)) != NULL;
	     csym = nsym->data) {

		if (*++kk == NULL)
			break;

		if (nsym->data == NULL) {	/* fatal error */
			free(nkey);
			splitstr_free(keys);
			return (ENOTDIR);
		}
		if (((SYM) (nsym->data))->magic != SYM_MAGIC) {
			free(nkey);
			splitstr_free(keys);
			return (ENOTDIR);
		}
	}

	if (*kk == NULL) {	/* found a complete match */
		free(nkey);
		splitstr_free(keys);

		if (flags == PUT_REPLACE) {
			nsym->data = data;
			return (0);
		} else {
			return (EEXIST);
		}
	}

	/* csym is a ptr to a list */
	for (; *kk != NULL; kk++) {
		if (*(kk + 1) != NULL) {
			add_key(csym, *kk, (void *)(ncsym = newsym()));
			csym = ncsym;
		} else {
			add_key(csym, *kk, data);	/* last key */
		}
	}

	free(nkey);
	splitstr_free(keys);
	return (0);
}

/*
 *	Retrieve a Symbol's Contents
 *
 *  "key" is not modified.
 *  If the key cannot be found, NULL is returned
 */
void *sym_get(SYM sym, char *key)
{
	char *nkey;
	const char **keys;	/* key split into a 2d string array */
	char **kk;
	SYM csym;		/* search: current symbol table */
	struct sym *nsym = NULL;	/* search: found symbol entry */

	if (sym == NULL)
		return (NULL);

	nkey = strdup(key);
	keys = splitstr(nkey, ",", NULL);
	if (keys == NULL)
		return (NULL);

	for (kk = (char **)keys, csym = sym;
	     *kk != NULL && (nsym = find_key1(csym->sym, *kk)) != NULL;
	     csym = nsym->data) {

		if (*++kk == NULL)
			break;

		if (nsym->data == NULL) {	/* fatal error */
			free(nkey);
			splitstr_free(keys);
			return (NULL);
		}
		if (((SYM) (nsym->data))->magic != SYM_MAGIC) {
			free(nkey);
			splitstr_free(keys);
			return (NULL);
		}
	}

	if (*kk == NULL) {	/* found a complete match */
		splitstr_free(keys);
		free(nkey);
		return (nsym->data);
	} else {
		splitstr_free(keys);
		free(nkey);
		return (NULL);
	}
}

/*
 *  Step thru a symbol table list
 *
 *  The cursor must be set by R_CURSOR, R_FIRST before using R_NEXT.
 *  NULL is returned when no more items are available.
 */
int sym_seq(SYM sym, DBT * key, DBT * data, int flags)
{
	SYM csym;

	switch (flags) {
		/*
		 * A number of ways to do this:
		 * specificly: sym_seq( .., "key,key") sets to Nth element of the 2nd
		 *  level symbol table
		 * sym_seq(.., "key,key,") sets to the first element of the 3rd
		 *  level symbol table
		 *
		 * sym_seq(.., "key,key") where both must be complete keys, sets
		 *  cursor to the first element of the 3rd level symbol table;
		 *  if there is no 3rd level, return an error.
		 */
	case R_CURSOR:
		csym = (SYM) sym_get(sym, (char *)key->data);
		if (csym == NULL || csym->magic != SYM_MAGIC) {
			return (2);
		}
		sym->cursor = csym->sym;
		if (sym->cursor == NULL)
			return (1);
		key->data = sym->cursor->key;
		data->data = sym->cursor->data;

		return (0);

	case R_FIRST:
		sym->cursor = sym->sym;
		if (sym->cursor == NULL)
			return (1);
		key->data = sym->cursor->key;
		data->data = sym->cursor->data;

		return (0);

	case R_NEXT:
		if (sym->cursor == NULL)
			return (1);
		sym->cursor = sym->cursor->next;

		if (sym->cursor == NULL)
			return (1);

		key->data = sym->cursor->key;
		data->data = sym->cursor->data;

		return (0);

	case R_LAST:
	case R_PREV:
	default:
		return (-1);
	}
}

/*
 *	Dump a symbol table's keys.
 *	Handles hierarchies, using a double quote to indicate depth, one
 *	double quote for each level.
 */
int sym_dump(SYM sym, int depth)
{

	register struct sym *se;	/* symbol entry */
	register int d;

	if (sym == NULL || sym->magic != SYM_MAGIC)
		return -1;

	for (se = sym->sym; se != NULL; se = se->next) {
		for (d = 0; d < depth; d++) {
			putchar('"');
			putchar(' ');
		}
		printf("%s\n", se->key);
		sym_dump((SYM) se->data, depth + 1);
	}
	return 0;
}

/*
 * sym dump, but data is _always_ a string (print it)
 */
int sym_dump_s(SYM sym, int depth)
{

	register struct sym *se;	/* symbol entry */
	register int d;

	if (sym == NULL)
		return 0;

	if (sym->magic != SYM_MAGIC) {
		for (d = 0; d < depth; d++) {
			putchar('"');
			putchar(' ');
		}
		printf(" = %s\n", (char *)sym);
		return 0;
	}

	for (se = sym->sym; se != NULL; se = se->next) {
		for (d = 0; d < depth; d++) {
			putchar('"');
			putchar(' ');
		}
		printf("%s", se->key);
		if (((SYM) se->data)->magic == SYM_MAGIC) {
			putchar('\n');
			sym_dump_s((SYM) se->data, depth + 1);
		} else {
			printf("(%p) = %s (%p)\n", se->key, (char *)se->data,
			       se->data);
		}
	}
	return 0;
}

/*
 *	Remove an entire symbol table (done bottom up)
 */
int sym_rm(SYM sym, int flags)
{
	register struct sym *se, *nse;	/* symbol entry */

	if (sym == NULL)
		return 0;

	if (sym->magic != SYM_MAGIC) {
		if (!(flags & RM_DATA))
			free(sym);
		return 0;
	}

	for (se = sym->sym; se != NULL;) {
		sym_rm((SYM) se->data, flags);
		nse = se->next;
		if (flags & RM_KEY)
			free(se->key);
		if (flags & RM_DATA)
			free(se->data);
		free(se);
		se = nse;
	}
	if (!(flags & RM_DATA))
		free(sym);
	return 0;
}
