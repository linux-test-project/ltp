/*
 *  Copyright (c) 2005 Silicon Graphics, Inc.
 *  All rights reserved.
 *
 *    Version 2 Jun 2005
 *
 *    Ray Bryant <raybry@sgi.com>
 *
 * Additions:  Lee Schermerhorn <lee.schermerhorn@hp.com>
 */

#ifdef _NEED_MIGRATE_PAGES
#include <sys/types.h>

#include <stdio.h>
#include <unistd.h>             /* For __NR_ni_syscall */
#include <errno.h>

/*
 * syscall numbers keep changings as we track newer kernels.
TODO:  x86_64 testing -- arch dependent sys call #s
 */
#define OLD_migrate_pages 1279	/* <= 2.6.13 */
#ifndef __NR_migrate_pages
#define __NR_migrate_pages 1280 // OLD_migrate_pages
#endif

int
migrate_pages(const pid_t pid, int count, unsigned int *old_nodes, unsigned int *new_nodes)
{
	int ret;

	ret = syscall(__NR_migrate_pages, pid, count, old_nodes, new_nodes);

	/*
	 *  If not implemented, maybe is because we're running a newer build
	 *  of program on older kernel?  Try the old sys call #
	 */
	if (ret == ENOSYS && __NR_migrate_pages != OLD_migrate_pages) {
fprintf(stderr, "%s:  trying old sys call # %d\n", __FUNCTION__, OLD_migrate_pages);
		ret = syscall(OLD_migrate_pages, pid, count, old_nodes, new_nodes);
	}

// fprintf(stderr, "%s:  migrate_pages() returns  %d, errno = %d\n", __FUNCTION__, ret, errno);
	return ret;
}
#endif
