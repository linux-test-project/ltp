/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Copyright (c) 2012, Cyril Hrubis <chrubis@suse.cz>
 *
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * The mmap() function shall fail if:
 * [EOVERFLOW] The file is a regular file and the value of off
 * plus len exceeds the offset maximum established in the open
 * file description associated with fildes.
 *
 * Test Steps:
 * 1. Set off and let to ULONG_MAX (make them align to page_size
 * 2. Assume the offset maximum is ULONG_MAX (on both 32 and 64 system).
 *
 * FIXME: Not quite sure how to make "the value of off plus len
 * exceeds the offset maxium established in the open file description
 * associated with files".
 *
 */

#define _XOPEN_SOURCE 600

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
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
	size_t len;
	int fd;
	off_t off = 0;

	long page_size = sysconf(_SC_PAGE_SIZE);

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_mmap_31_1_%d", getpid());
	unlink(tmpfname);
	fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf("Error at open(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}
	unlink(tmpfname);

	/*
	 * len + off > maximum offset
	 * FIXME: We assume maximum offset is ULONG_MAX
	 */
	len = ULONG_MAX;
	if (len % page_size) {
		/* Lower boundary */
		len &= ~(page_size - 1);
	}

	off = ULONG_MAX;
	if (off % page_size) {
		/* Lower boundary */
		off &= ~(page_size - 1);
	}

	printf("off: %lx, len: %lx\n", (unsigned long)off, (unsigned long)len);

	pa = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, off);
	if (pa == MAP_FAILED && errno == EOVERFLOW) {
		printf("Got EOVERFLOW\n");
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	if (pa == MAP_FAILED)
		perror("Test FAILED: expect EOVERFLOW but get other error");
	else
		printf("Test FAILED: Expect EOVERFLOW but got no error\n");

	close(fd);
	munmap(pa, len);
	return PTS_FAIL;
}
