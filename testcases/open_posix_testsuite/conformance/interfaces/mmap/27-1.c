/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Copyright (c) 2012, Cyril Hrubis <chrubis@suse.cz>
 *
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * The mmap() function shall fail if:
 * [ENOTSUP] MAP_FIXED or MAP_PRIVATE was specified
 * in the flags argument and the
 * implementation does not support this functionality.
 * The implementation does not support the combination
 * of accesses requested in the prot argument.
 *
 * Test Steps:
 * 1. Try mmap with MAP_PRIVATE should either fail with ENOTSUP or SUCCEED
 * 2. Try fixed mapping
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
	int total_size = 1024;

	void *pa;
	size_t len = total_size;
	int fd, err = 0;

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_mmap_27_1_%d", getpid());
	unlink(tmpfname);
	fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf("Error at open(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}

	/* Make sure the file is removed when it is closed */
	unlink(tmpfname);

	if (ftruncate(fd, total_size) == -1) {
		printf("Error at ftruncate(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}

	/* Trie to map file with MAP_PRIVATE */
	pa = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
	if (pa == MAP_FAILED) {
		if (errno != ENOTSUP) {
			printf("MAP_PRIVATE is not supported\n");
		} else {
			printf("MAP_PRIVATE failed with: %s\n",
			       strerror(errno));
			err++;
		}
	} else {
		printf("MAP_PRIVATE succeeded\n");
		munmap(pa, len);
	}

	/* Now try to utilize MAP_FIXED */
	pa = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (pa == MAP_FAILED) {
		printf("Error at mmap(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}

	pa = mmap(pa, len / 2, PROT_READ, MAP_SHARED | MAP_FIXED, fd, 0);

	if (pa == MAP_FAILED) {
		if (errno != ENOTSUP) {
			printf("MAP_FIXED is not supported\n");
		} else {
			printf("MAP_FIXED failed with: %s\n", strerror(errno));
			err++;
		}
	} else {
		printf("MAP_FIXED succeeded\n");
		munmap(pa, len);
	}

	if (err)
		return PTS_FAIL;

	printf("Test PASSED\n");
	return PTS_PASS;
}
