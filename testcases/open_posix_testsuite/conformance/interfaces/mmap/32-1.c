/*
 * Copyright (c) 2012, Cyril Hrubis <chrubis@suse.cz>
 *
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * If len is zero, mmap() shall fail and no mapping shall be established.
 */


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
#include "tempfile.h"

int main(void)
{
	char tmpfname[PATH_MAX];

	void *pa;
	int fd;

	PTS_GET_TMP_FILENAME(tmpfname, "pts_mmap_32_1");
	unlink(tmpfname);
	fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf("Error at open(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}
	unlink(tmpfname);

	pa = mmap(NULL, 0, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (pa == MAP_FAILED && errno == EINVAL) {
		printf("Got EINVAL\n");
		printf("Test PASSED\n");
		close(fd);
		return PTS_PASS;
	}

	if (pa == MAP_FAILED)
		printf("Test FAILED: Expected EINVAL got %s\n", strerror(errno));
	else
		printf("Test FAILED: mmap() succedded unexpectedly\n");

	close(fd);
	munmap(pa, 0);
	return PTS_FAIL;
}
