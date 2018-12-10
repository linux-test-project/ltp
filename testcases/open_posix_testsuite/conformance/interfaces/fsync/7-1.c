/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * The fsync() function shall fail if:
 * [EINVAL] The fildes argument does not refer to a file
 * on which this operation is possible.
 *
 * Test Step:
 * 1. Create a pipe;
 * 2. fsync on the pipe, should fail with EINVAL;
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

#define TNAME "fsync/7-1.c"

int main(void)
{
	int fd[2];

	if (pipe(fd) == -1) {
		printf(TNAME " Test UNRESOLVED: Error at pipe: %s\n",
		       strerror(errno));
		exit(PTS_UNRESOLVED);
	}

	if (fsync(fd[1]) == -1 && errno == EINVAL) {
		printf("Got EINVAL when fsync on pipe\n");
		printf("Test PASSED\n");
		close(fd[0]);
		close(fd[1]);
		exit(PTS_PASS);
	} else {
		printf(TNAME " Test Fail: Expect EINVAL, get: %s\n",
		       strerror(errno));
		close(fd[0]);
		close(fd[1]);
		exit(PTS_FAIL);
	}
}
