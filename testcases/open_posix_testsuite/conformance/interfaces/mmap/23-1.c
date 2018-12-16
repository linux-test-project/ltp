/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Copyright (c) 2012, Cyril Hrubis <chrubis@suse.cz>
 *
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * The mmap() function shall fail if: [ENODEV] The fildes argument refers to a
 * file whose type is not supported by mmap().
 *
 * Test Steps:
 * 1. Create pipe;
 * 2. mmap the pipe fd to memory, should get ENODEV;
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
	int pipe_fd[2];
	void *pa;

	if (pipe(pipe_fd) == -1) {
		printf("Error at pipe(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}

	pa = mmap(NULL, 1024, PROT_READ, MAP_SHARED, pipe_fd[0], 0);

	if (pa == MAP_FAILED && errno == ENODEV) {
		printf("Test PASSED\n");
		close(pipe_fd[0]);
		close(pipe_fd[1]);
		return PTS_PASS;
	}

	if (pa == MAP_FAILED) {
		printf("Test FAILED: Expect ENODEV, get: %s\n",
		       strerror(errno));
	} else {
		printf("Text FAILED: mmap() succeded\n");
	}

	close(pipe_fd[0]);
	close(pipe_fd[1]);

	return PTS_FAIL;
}
