/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Copyright (c) 2012, Cyril Hrubis <chrubis@suse.cz>
 *
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * When the implementation selects a
 * The mmap() function shall fail if:
 * [EINVAL] The value of flags is invalid (neither MAP_PRIVATE nor MAP_SHARED is
 * set).
 *
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

	void *pa;
	size_t size = 1024;
	int flag = ~0;
	int fd;

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_mmap_21_1_%d", getpid());
	unlink(tmpfname);
	fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf("Error at open(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}
	unlink(tmpfname);

	if (ftruncate(fd, size) == -1) {
		printf("Error at ftruncate(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}

	pa = mmap(NULL, size, PROT_READ | PROT_WRITE, flag, fd, 0);
	if (pa == MAP_FAILED && errno == EINVAL) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	close(fd);
	munmap(pa, size);
	printf("Test FAILED\n");
	return PTS_FAIL;
}
