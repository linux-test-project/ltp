
#ifndef _DBLLISTS_H
#define _DBLLISTS_H



#define DBL_LIST_HEAD_INIT(name)        { &(name), &(name) }

#define DBL_LIST_HEAD(name)             struct list_head name = DBL_LIST_HEAD_INIT(name)

#define DBL_INIT_LIST_HEAD(ptr) \
do { \
    (ptr)->pNext = (ptr)->pPrev = (ptr); \
} while (0)

#define DBL_LIST_ADD(newi, prev, next) \
do { \
    struct list_head *pPrev = prev; \
    struct list_head *pNext = next; \
    (pNext)->pPrev = newi; \
    (newi)->pNext = pNext; \
    (newi)->pPrev = pPrev; \
    (pPrev)->pNext = newi; \
} while (0)

#define DBL_LIST_ADDH(new, head)        DBL_LIST_ADD(new, head, (head)->pNext)

#define DBL_LIST_ADDT(new, head)        DBL_LIST_ADD(new, (head)->pPrev, head)

#define DBL_LIST_UNLINK(prev, next) \
do { \
    struct list_head *pPrev = prev; \
    struct list_head *pNext = next; \
    (next)->pPrev = pPrev; \
    (prev)->pNext = pNext; \
} while (0)

#define DBL_LIST_DEL(entry) \
do { \
    DBL_LIST_UNLINK((entry)->pPrev, (entry)->pNext); \
    DBL_INIT_LIST_HEAD(entry); \
} while (0)

#define DBL_LIST_EMTPY(head)            ((head)->pNext == head)

#define DBL_LIST_SPLICE(list, head) \
do { \
    struct list_head *    first = (list)->pNext; \
    if (first != list) { \
        struct list_head *	last = (list)->pPrev; \
        struct list_head *	at = (head)->pNext; \
        (first)->pPrev = head; \
        (head)->pNext = first; \
        (last)->pNext = at; \
        (at)->pPrev = last; \
    } \
} while (0)

#define DBL_HEAD_COPY(oldh, newh) \
do { \
    *(oldh) = (*newh); \
    (newh)->pNext->pPrev = (oldh); \
    (newh)->pPrev->pNext = (oldh); \
} while (0)

#define DBL_ITEM_IN_LIST(ptr)           (((ptr)->pPrev != (ptr)) && ((ptr)->pNext != (ptr)))

#define DBL_LIST_FIRST(head)            (((head)->pNext != (head)) ? (head)->pNext: NULL)

#define DBL_LIST_LAST(head)             (((head)->pPrev != (head)) ? (head)->pPrev: NULL)

#define DBL_LIST_ENTRY(ptr, type, member)   ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))

#define DBL_LIST_FOR_EACH(pos, head)    for (pos = (head)->pNext; pos != (head); pos = (pos)->pNext)

#define DBL_END_OF_LIST(pos, head)      ((pos) == (head))

#define DBL_LIST_NEXT(pos, head)        (((pos)->pNext != (head)) ? (pos)->pNext: NULL)

#define DBL_LIST_PREV(pos, head)        (((pos)->pPrev != (head)) ? (pos)->pPrev: NULL)







struct list_head
{
    struct list_head *pNext;
    struct list_head *pPrev;
};





#endif
