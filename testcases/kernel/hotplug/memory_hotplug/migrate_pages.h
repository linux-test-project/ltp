/*
 * memtoy - migrate_pages.h - interface to migrate_pages() system call
 *
 * until merged into upstream kernels [he says hopefully].
 */
#ifndef _MEMTOY_MIGRATE_PAGES_
#define _MEMTOY_MIGRATE_PAGES_

#ifdef _NEED_MIGRATE_PAGES
extern int migrate_pages(const pid_t, int, unsigned int*, unsigned int*);
#endif

// temporary?  
#ifndef MPOL_MF_MOVE
#define MPOL_MF_MOVE    (1<<1)  /* Move existing pages, if possible */
#endif
#ifndef MPOL_MF_WAIT
#define MPOL_MF_WAIT    (1<<2)  /* Wait for existing pages to migrate */
#endif

#endif
