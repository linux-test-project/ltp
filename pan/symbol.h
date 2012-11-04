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
/* $Id: symbol.h,v 1.1 2000/09/21 21:35:06 alaffin Exp $ */
#ifndef _SYMBOL_H_
#define _SYMBOL_H_

/*
 *	"Generic" Symbol Table
 *
 *  These data structures are the internal part of a library providing
 *  an in-memory dbm-like (key, content) database with hierarchical
 *  key names.
 */
struct sym {
    struct sym *next;
    char       *key;
    void       *data;
};

/*
 * Symbol Table Header
 */
struct symh {
    int         magic;
    struct sym *sym;
    struct sym *cursor;
};

/*
 * The "SYM" typedef is the only external data type.
 */
typedef struct symh *SYM;

/*
 * Data for keys and contents (lifted from dbopen(3))
 * dbopen(3) uses this for all functions, but I'm hard-wired into chars
 * for keys and the like; I just need this for sym_get
 */
typedef struct {
    void *data;
    int   size;
} DBT;

/*
 * Prototypes
 */

SYM   sym_open(int flags, int mode,  int openinfo          );
int   sym_put (SYM sym,   char *key, void *data, int flags );
void *sym_get (SYM sym,   char *key                        );
int   sym_seq (SYM sym,   DBT *key,  DBT *data, int flags  );
int   sym_rm  (SYM sym,   int flags                        );

/*
 * Flags for sym_put
 */
#define PUT_REPLACE	1	/* replace data on a put */

/*
 * Flags for sym_rm
 */
#define	RM_KEY	001		/* free() on key pointer */
#define	RM_DATA	002		/* free() on data pointer */

/*
 * Flags for sym_seq (clones of 44BSD dbopen(3))
 */
#define	R_CURSOR	1	/* set "cursor" to where "data" key is */
#define R_FIRST		2	/* set "cursor" to first item */
#define	R_NEXT		4	/* set "cursor" to next item */
#define	R_LAST		3	/* set "cursor" to last item (UNIMP) */
#define	R_PREV		5	/* set "cursor" to previous item (UNIMP) */

#endif
