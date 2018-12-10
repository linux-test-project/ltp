/*
 * Copyright (c) 2004, Bull SA. All rights reserved.
 * Created by:  Laurent.Vivier@bull.net
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
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

#define TNAME "aio_fsync/12-1.c"

int main(void)
{
	struct aiocb aiocb;

	if (sysconf(_SC_ASYNCHRONOUS_IO) < 200112L)
		return PTS_UNSUPPORTED;

	memset(&aiocb, 0, sizeof(struct aiocb));
	aiocb.aio_fildes = -1;

	if (aio_fsync(O_SYNC, &aiocb) != -1) {
		printf(TNAME " aio_fsync() accepts bad filedes\n");
		exit(PTS_FAIL);
	}

	if (errno != EBADF) {
		printf(TNAME " errno is not EBADF (%d)\n", errno);
		exit(PTS_FAIL);
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
