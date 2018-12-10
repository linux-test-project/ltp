/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * The fsync() function shall fail if:
 * [EBADF] The fildes argument is not a valid descriptor.
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
	int fd;

	/* -1 is an invalid fd */

	fd = -1;
	if (fsync(fd) == -1 && errno == EBADF) {
		printf("Got EBADF when fd=-1\n");
		printf("Test PASSED\n");
		exit(PTS_PASS);
	} else {
		printf("Test FAILED: Expect EBADF, get: %s\n", strerror(errno));
		exit(PTS_FAIL);
	}
}
