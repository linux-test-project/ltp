/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Copyright (c) 2012, Cyril Hrubis <chrubis@suse.cz>
 *
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * The mmap() function shall establish a mapping between a process's
 * address space and a file,
 *
 * Test Steps:
 * 1. Create a tmp file;
 * 2. mmap it to memory using mmap();
 *
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
	ssize_t len = 1024;
	char data[len];
	void *pa;
	int fd;

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_mmap_1_1_%d", getpid());
	unlink(tmpfname);
	fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf("Error at open(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}

	unlink(tmpfname);

	memset(data, 'a', len);
	if (write(fd, data, len) != len) {
		printf("Error at write(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}

	pa = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (pa == MAP_FAILED) {
		printf("Error at mmap: %s\n", strerror(errno));
		return PTS_FAIL;
	}

	if (*(char *)pa != 'a') {
		printf("Test FAILED: The file was not mapped correctly.\n");
		return PTS_FAIL;
	}

	close(fd);
	munmap(pa, len);
	printf("Test PASSED\n");
	return PTS_PASS;
}
