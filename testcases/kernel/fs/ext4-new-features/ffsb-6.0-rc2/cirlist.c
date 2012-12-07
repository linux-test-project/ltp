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
#include <stdlib.h>
#include <assert.h>

#include "cirlist.h"
#include "util.h"

void init_cirlist(struct cirlist *cl)
{
	cl->count = 0;
	cl->head = NULL;
}

int cl_empty(struct cirlist *cl)
{
	return !(cl->count);
}

void cl_insert_tail(struct cirlist *cl, cldatatype object)
{
	struct cnode *new = ffsb_malloc(sizeof(struct cnode));
	new->obj = object;
	if (cl->count == 0) {
		assert(cl->head == NULL);
		cl->head = new;
		cl->head->next = cl->head;
		cl->head->prev = cl->head;
		cl->count = 1;
	} else {
		if (cl->count == 1) {
			assert(cl->head->next == cl->head);
			assert(cl->head->prev == cl->head);
			cl->head->next = new;
			cl->head->prev = new;
			new->next = cl->head;
			new->prev = cl->head;
		} else {
			assert(cl->head->next != cl->head);
			assert(cl->head->prev != cl->head);

			new->next = cl->head;
			new->prev = (cl->head)->prev;
			cl->head->prev->next = new;
			cl->head->prev = new;
		}
		cl->count++;
	}
}

cldatatype cl_remove_head(struct cirlist *cl)
{
	struct cnode *oldhead = NULL;
	struct cnode *newhead = NULL;
	cldatatype ret = NULL;

	if (cl->count == 0) {
		assert(cl->head == NULL);
		return NULL;
	}
	if (cl->count == 1) {
		assert(cl->head->next == cl->head);
		assert(cl->head->prev == cl->head);
		oldhead = cl->head;
		cl->head = NULL;
		cl->count = 0;
	} else if (cl->count == 2) {
		oldhead = cl->head;
		newhead = oldhead->next;
		newhead->next = newhead;
		newhead->prev = newhead;
		cl->head = newhead;
		cl->count = 1;
	} else {
		assert(cl->head->next != cl->head);
		assert(cl->head->prev != cl->head);
		oldhead = cl->head;
		newhead = oldhead->next;
		newhead->prev = oldhead->prev;
		newhead->prev->next = newhead;
		cl->head = newhead;
		cl->count--;
	}
	ret = oldhead->obj;
	oldhead->obj = (void *)(-1);
	oldhead->next = (void *)(-1);
	oldhead->prev = (void *)(-1);
	free(oldhead);

	return ret;
}
