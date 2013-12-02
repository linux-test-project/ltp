/*
 *   Copyright (c) International Business Machines Corp., 2001-2004
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
#ifndef _CIRLIST_H
#define _CIRLIST_H

#include "filelist.h"

typedef struct ffsb_file *cldatatype;

struct cnode {
	cldatatype obj;
	struct cnode *next;
	struct cnode *prev;
};

struct cirlist {
	int count;
	struct cnode *head;
};

void init_cirlist(struct cirlist *cl);
int cl_empty(struct cirlist *cl);
void cl_insert_tail(struct cirlist *cl , cldatatype object);
cldatatype cl_remove_head(struct cirlist *cl);

#endif /* _CIRLIST_H */
