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
 *	The aio_write() function shall return the value -1 and set errno to
	indicate error if the operation is not successfully queued.
 *
 * method:
 *
 *	- if Prioritized Input and Output option is supported, fill in an
 *	  aiocb with invalid aio_reqprio
 *	- call aio_write
 *	- check aio_write return value
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

#define TNAME "aio_write/6-1.c"

int main(void)
{
	struct aiocb aiocb;

	if (sysconf(_SC_ASYNCHRONOUS_IO) < 200112L)
		return PTS_UNSUPPORTED;

	if (sysconf(_SC_PRIORITIZED_IO) < 200112L)
		return PTS_UNTESTED;

	memset(&aiocb, 0, sizeof(struct aiocb));
	aiocb.aio_reqprio = -1;

	if (aio_write(&aiocb) != -1) {
		printf(TNAME " aio_write should fail!\n");
		exit(PTS_FAIL);
	}

	if (errno != EINVAL) {
		printf(TNAME " errno should be EINVAL!\n");
		exit(PTS_FAIL);
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
