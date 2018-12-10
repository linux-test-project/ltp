/*
 * Copyright (c) 2004, Bull SA. All rights reserved.
 * Created by:  Laurent.Vivier@bull.net
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/* assertion:
 *
 *	aio_cancel() shall fail if:
 *	[EBADF] The fildes argument is not a valid descriptor.
 *
 * method:
 *
 *	use -1 as fildes and check return value is -1 and errno is EBADF
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <aio.h>

#include "posixtest.h"

#define TNAME "aio_cancel/10-1.c"

int main(void)
{

	if (sysconf(_SC_ASYNCHRONOUS_IO) < 200112L)
		return PTS_UNSUPPORTED;

	if (aio_cancel(-1, NULL) != -1) {
		printf(TNAME " bad aio_cancel return value()\n");
		return PTS_FAIL;
	}

	if (errno != EBADF) {
		printf(TNAME " errno is not EBADF %s\n", strerror(errno));
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
