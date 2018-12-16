/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Copyright (c) 2012, Cyril Hrubis <chrubis@suse.cz>
 *
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * The mmap() function shall establish a mapping
 * between a process's address space
 * and a shared memory object.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "posixtest.h"

int main(void)
{
	char tmpfname[256];
	void *pa;
	size_t size = 1024 * 4 * 1024;
	int fd;

	snprintf(tmpfname, sizeof(tmpfname), "pts_mmap_1_2_%d", getpid());

	/* Create shared object */
	shm_unlink(tmpfname);
	fd = shm_open(tmpfname, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf("Error at shm_open(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}
	shm_unlink(tmpfname);
	if (ftruncate(fd, size) == -1) {
		printf("Error at ftruncate(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}

	if (write(fd, "a", 1) != 1) {
		printf("Error at write(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}

	pa = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (pa == MAP_FAILED) {
		printf("Error at mmap: %s\n", strerror(errno));
		return PTS_FAIL;
	}

	if (*(char *)pa != 'a') {
		printf("Test FAILED: The file was not mapped correctly.\n");
		return PTS_FAIL;
	}

	close(fd);
	munmap(pa, size);
	printf("Test PASSED\n");
	return PTS_PASS;
}
