
/*
 * Simple list implementation mostly take from the Linux Kernel
 */

#ifndef _LIST_H_
#define _LIST_H_

struct list_head {
	struct list_head *next, *prev;
};

void INIT_LIST_HEAD(struct list_head *list);
void __list_add(struct list_head *new, struct list_head *prev,
		struct list_head *next);
void __list_del(struct list_head *prev, struct list_head *next);
void list_add(struct list_head *new, struct list_head *head);
void list_add_tail(struct list_head *new, struct list_head *head);
void list_del(struct list_head *entry);
void list_replace(struct list_head *old, struct list_head *new);

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) struct list_head name = LIST_HEAD_INIT(name)

#define list_for_each(pos, head) \
	for (pos = (head)->next; pos->next, pos != (head); pos = pos->next)

#define offsetof(type, member) ((int) &((type *)0)->member)

#define container_of(ptr, type, member) ({ \
	const typeof(((type *)0)->member) *__mptr = (ptr); \
	(type *)((char *)__mptr - offsetof(type, member)); })

#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)

#define list_for_each_entry(pos, head, member) \
	for (pos = list_entry((head)->next, typeof(*pos), member);	\
	     pos->member.next, &pos->member != (head); 	\
	     pos = list_entry(pos->member.next, typeof(*pos), member))

#define list_for_each_entry_safe(pos, n, head, member)			\
	for (pos = list_entry((head)->next, typeof(*pos), member),	\
		n = list_entry(pos->member.next, typeof(*pos), member);	\
	     &pos->member != (head); 					\
	     pos = n, n = list_entry(n->member.next, typeof(*n), member))

#define list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); \
		pos = n, n = pos->next)

#endif
