/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Copyright (c) 2012, Cyril Hrubis <chrubis@suse.cz>
 *
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * The mmap() function shall fail if:
 * [EBADF] The fildes argument is not a valid open file descriptor.
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
	void *pa;
	int fd = -1;

	pa = mmap(NULL, 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	if (pa == MAP_FAILED && errno == EBADF) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	if (pa == MAP_FAILED)
		perror("mmap()");

	printf("Test FAILED: Did not get EBADF when fd is invalid\n");
	return PTS_FAIL;
}
