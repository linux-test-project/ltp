/*
 * Copyright (c) 2004, Bull SA. All rights reserved.
 * Copyright (c) 2026 SUSE LLC
 * Created by:  Laurent.Vivier@bull.net
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 *
 *	aio_cancel() shall return AIO_CANCELED if the requested operations
 *	were canceled.
 *
 * method:
 *
 *	queue multiple aio_write()s to a given socket
 *	try to cancel a task which hasn't been started yet
 *	if aio_cancel() return value is not AIO_CANCELED, the test failed
 *	for blocked tasks, aio_error() must be:
 *	- ECANCELED if aio_cancel() was called on it
 *	- EINPROGRESS otherwise
 *	if all aio_error() values match, the test passed, otherwise it failed
 *
 */

#include <unistd.h>

#include "posixtest.h"
#include "aio_test.h"

#define TNAME "aio_cancel/6-1.c"

#define WRITE_COUNT	8
#define MAX_COMPLETE	3
#define CANCELED_TASK	5

static int fds[2];
static struct aiocb aiocb[WRITE_COUNT];

int test_main(int argc PTS_ATTRIBUTE_UNUSED, char **argv PTS_ATTRIBUTE_UNUSED)
{
	int i;
	int gret;

	if (sysconf(_SC_ASYNCHRONOUS_IO) < 200112L)
		return PTS_UNSUPPORTED;

	if (setup_aio(TNAME, fds, aiocb, WRITE_COUNT))
		return PTS_UNRESOLVED;

	/* create AIO req */
	for (i = 0; i < WRITE_COUNT; i++) {
		if (aio_write(&aiocb[i]) == -1) {
			printf(TNAME " loop %d: Error at aio_write(): %s\n",
				i, strerror(errno));
			cleanup_aio(fds, aiocb, WRITE_COUNT);
			return PTS_FAIL;
		}
	}

	gret = aio_cancel(fds[0], &aiocb[CANCELED_TASK]);

	if (gret == -1) {
		printf(TNAME " Error at aio_cancel(): %s\n", strerror(errno));
		cleanup_aio(fds, aiocb, WRITE_COUNT);
		return PTS_FAIL;
	}

	if (gret != AIO_CANCELED) {
		printf(TNAME " Unexpected aio_cancel() return value %d\n",
			gret);
		cleanup_aio(fds, aiocb, WRITE_COUNT);
		return PTS_FAIL;
	}

	for (i = MAX_COMPLETE; i < WRITE_COUNT; i++) {
		int exp_ret = (i == CANCELED_TASK) ? ECANCELED : EINPROGRESS;
		int ret = aio_error(&aiocb[i]);

		if (ret == -1) {
			printf(TNAME " Error at aio_error(): %s\n",
				strerror(errno));
			cleanup_aio(fds, aiocb, WRITE_COUNT);
			return PTS_FAIL;
		}

		if (ret != exp_ret) {
			printf(TNAME " Bad task #%d result %s",
				i, strerror(ret));
			printf(" (expected: %s)\n", strerror(exp_ret));
			cleanup_aio(fds, aiocb, WRITE_COUNT);
			return PTS_FAIL;
		}
	}

	cleanup_aio(fds, aiocb, WRITE_COUNT);
	printf("Test PASSED\n");
	return PTS_PASS;
}
