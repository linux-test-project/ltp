
/*
 * Simple list implementation mostly take from the Linux Kernel
 */

#include <stdlib.h>
#include "list.h"

void INIT_LIST_HEAD(struct list_head *list)
{
	list->next = list;
	list->prev = list;
}

void __list_add(struct list_head *new,
		struct list_head *prev, struct list_head *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

void __list_del(struct list_head *prev, struct list_head *next)
{
	next->prev = prev;
	prev->next = next;
}

void list_add(struct list_head *new, struct list_head *head)
{
	__list_add(new, head, head->next);
}

void list_add_tail(struct list_head *new, struct list_head *head)
{
	__list_add(new, head->prev, head);
}

void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = NULL;
	entry->prev = NULL;
}

void list_replace(struct list_head *old, struct list_head *new)
{
	new->next = old->next;
	new->next->prev = new;
	new->prev = old->prev;
	new->prev->next = new;
}
