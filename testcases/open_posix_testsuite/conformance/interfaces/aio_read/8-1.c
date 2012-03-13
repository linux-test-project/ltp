/*
 * Copyright (c) 2004, Bull SA. All rights reserved.
 * Created by:  Laurent.Vivier@bull.net
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 *
 * The aio_read() function shall return the value -1 and set errno to
 * indicate error if the operation is not succesfully queued.
 *
 * method:
 *
 *	- fill in an aiocb with a NULL aio_buf
 *	- call aio_read
 *	- check aio_read return value
 */

#define _XOPEN_SOURCE 600
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

#define TNAME "aio_read/8-1.c"

int main()
{
	struct aiocb aiocb;

	if (sysconf(_SC_ASYNCHRONOUS_IO) < 200112L)
		return PTS_UNSUPPORTED;

	/* submit a request with a NULL buffer */
	aiocb.aio_fildes = 0;
	aiocb.aio_buf = NULL;
	aiocb.aio_nbytes = 0;
	aiocb.aio_offset = 0;

	if (aio_read(&aiocb) != -1) {
		printf(TNAME " aio_read() should fail!\n");
		exit(PTS_FAIL);
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
