/* Manage a list of processes. */

/*
 * Copyright (C) 2003-2006 IBM
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <malloc.h>
#include "proclist.h"

void add_to_proclist(struct proclist_t *list, struct proclist_item_t *item)
{
	struct proclist_item_t *curr;

	if (list->head == NULL) {
		item->next = NULL;
		list->head = item;
		return;
	}

	curr = list->head;
	while (curr->next != NULL) {
		curr = curr->next;
	}

	item->next = NULL;
	curr->next = item;
}

void remove_from_proclist(struct proclist_t *list, struct proclist_item_t *item)
{
	struct proclist_item_t *curr, *prev;

	if (list->head == NULL) {
		return;
	}

	if (list->head == item) {
		list->head = item->next;
		item->next = NULL;
		return;
	}

	prev = list->head;
	curr = list->head->next;

	while (curr != NULL && curr != item) {
		prev = curr;
		curr = curr->next;
	}

	if (curr == NULL) {
		return;
	}

	prev->next = item->next;
	item->next = NULL;
}
