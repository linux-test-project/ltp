/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Copyright (c) 2012, Cyril Hrubis <chrubis@suse.cz>
 *
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * If MAP_FIXED is set,
 * mmap() may return MAP_FAILED and set errno to [EINVAL].
 *
 * [EINVAL] The addr argument (if MAP_FIXED was specified) or off is not a
 * multiple of the page size as returned by sysconf(), or is considered invalid
 * by the implementation.
 *
 * Test Steps:
 * 1. Set 'addr' as an illegal address, which is not a multiple of page size;
 * 2. Call mmap() and get EINVAL;
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "posixtest.h"

int main(void)
{
	char tmpfname[256];
	long page_size;
	long total_size;

	void *illegal_addr;
	void *pa;
	size_t size;
	int fd, saved_errno;

	page_size = sysconf(_SC_PAGE_SIZE);
	total_size = page_size;
	size = total_size;

	/* Create tmp file */
	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_mmap_9_1_%d", getpid());
	unlink(tmpfname);
	fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf("Error at open(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}
	unlink(tmpfname);
	if (ftruncate(fd, total_size) == -1) {
		printf("Error at ftruncate(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}

	/* Map the file for the first time, to get a legal address, pa */
	pa = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	if ((unsigned long)pa % page_size) {
		printf("pa is not multiple of page_size\n");
		illegal_addr = pa;
	} else {
		printf("pa is a multiple of page_size\n");
		illegal_addr = pa + 1;
	}

	munmap(pa, size);

	/* Mmap again using the illegal address, setting MAP_FIXED */
	pa = mmap(illegal_addr, size, PROT_READ | PROT_WRITE, MAP_FIXED, fd, 0);

	saved_errno = errno;

	close(fd);
	munmap(pa, size);

	if (pa == MAP_FAILED && saved_errno == EINVAL) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	printf("Test FAILED, mmap with MAP_FIXED did not get EINVAL"
	       " when 'addr' is illegal\n");
	return PTS_FAIL;
}
