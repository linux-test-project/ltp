/* Declarations to manage a list of processes. */

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

#ifndef PROCLIST_H_
#define PROCLIST_H_

#include <sys/types.h>

struct proclist_item_t {
	struct proclist_item_t *next;
	pid_t pid;
	char *name;
};

struct proclist_t {
	struct proclist_item_t *head;
};

void add_to_proclist(struct proclist_t *list, struct proclist_item_t *item);
void remove_from_proclist(struct proclist_t *list, struct proclist_item_t *item);

#endif
