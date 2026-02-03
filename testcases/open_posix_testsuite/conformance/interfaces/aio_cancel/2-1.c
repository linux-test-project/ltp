/*
 * Copyright (c) 2004, Bull SA. All rights reserved.
 * Copyright (c) 2025 SUSE LLC
 * Created by:  Laurent.Vivier@bull.net
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 *
 *	if aiobcp is NULL, all outstanding cancelable AIO against fildes
 *	shall be canceled.
 *
 * method:
 *
 *	open a pair of sockets and queue writes to them with aio_write()
 *	execute aio_cancel() on the socket
 *	check aio_cancel() returns zero or AIO_NOTCANCELED
 *	check that aio_error() returns ECANCELED for cancelable requests
 *	-> aio_cancel() works on fildes used with an aio command
 *
 *	we queue enough writes to ensure that one of them will block
 *	on full socket buffer and the last one will be cancelable
 *
 */

#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#include "posixtest.h"
#include "aio_test.h"

#define TNAME "aio_cancel/2-1.c"

#define WRITE_COUNT	8
#define BLOCKED_TASK	2

static int fds[2];
static struct aiocb aiocb[WRITE_COUNT];

int main(void)
{
	int i;
	int ret;

	if (sysconf(_SC_ASYNCHRONOUS_IO) < 200112L)
		return PTS_UNSUPPORTED;

	if (setup_aio(TNAME, fds, aiocb, WRITE_COUNT))
		return PTS_UNRESOLVED;

	/* submit AIO req */
	for (i = 0; i < WRITE_COUNT; i++) {
		if (aio_write(&aiocb[i]) == -1) {
			printf(TNAME " loop %d: Error at aio_write(): %s\n",
			       i, strerror(errno));
			cleanup_aio(fds, aiocb, WRITE_COUNT);
			return PTS_FAIL;
		}
	}

	ret = aio_cancel(fds[0], NULL);

	if (ret == -1) {
		printf(TNAME " Error at aio_cancel(): %s\n", strerror(errno));
		cleanup_aio(fds, aiocb, WRITE_COUNT);
		return PTS_FAIL;
	}

	if (ret != 0 && ret != AIO_NOTCANCELED) {
		printf(TNAME " Unexpected aio_cancel() return value: %d\n",
			ret);
		cleanup_aio(fds, aiocb, WRITE_COUNT);
		return PTS_FAIL;
	}

	for (i = BLOCKED_TASK + 1; i < WRITE_COUNT; i++) {
		ret = aio_error(&aiocb[i]);

		if (ret != ECANCELED) {
			printf(TNAME " AIO request %d was not canceled: %s\n",
				i, strerror(ret));
			cleanup_aio(fds, aiocb, WRITE_COUNT);
			return PTS_FAIL;
		}
	}

	cleanup_aio(fds, aiocb, WRITE_COUNT);
	printf("Test PASSED\n");
	return PTS_PASS;
}
