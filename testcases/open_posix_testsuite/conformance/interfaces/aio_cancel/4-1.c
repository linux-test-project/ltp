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
 *	Error status should be set to [ECANCELED] and return shall be -1 for
 *	successfully canceled AIO.
 *
 * method:
 *
 *	open a pair of sockets and queue writes to them with aio_write()
 *	execute aio_cancel() on the socket
 *	check aio_error() and aio_return() results for writes which should
 *	have been canceled
 *	if any aio_error() is not ECANCELED or aio_return() is not -1,
 *	the test fails
 *	otherwise the test passes
 *
 */

#include <sys/types.h>
#include <unistd.h>

#include "posixtest.h"
#include "aio_test.h"

#define TNAME "aio_cancel/4-1.c"

#define WRITE_COUNT	8
#define MAX_COMPLETE	3

static int fds[2];
static struct aiocb aiocb[WRITE_COUNT];

int main(void)
{
	int i;

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

	/* cancel all */
	if (aio_cancel(fds[0], NULL) == -1) {
		printf(TNAME " Error at aio_cancel(): %s\n", strerror(errno));
		cleanup_aio(fds, aiocb, WRITE_COUNT);
		return PTS_FAIL;
	}

	/* check results of requests that should have been canceled */
	for (i = MAX_COMPLETE; i < WRITE_COUNT; i++) {
		int ret = aio_error(&aiocb[i]);

		if (ret == -1) {
			printf(TNAME " Error at aio_error(): %s\n",
			       strerror(errno));
			cleanup_aio(fds, aiocb, WRITE_COUNT);
			return PTS_FAIL;
		} else if (ret != ECANCELED) {
			printf(TNAME " Bad task #%d result: %s "
				"(expected ECANCELED)\n", i, strerror(ret));
			cleanup_aio(fds, aiocb, WRITE_COUNT);
			return PTS_FAIL;
		}

		ret = aio_return(&aiocb[i]);

		if (ret != -1) {
			printf(TNAME " aio_return(): %d (expected -1)\n", ret);
			cleanup_aio(fds, aiocb, WRITE_COUNT);
			return PTS_FAIL;
		}
	}

	cleanup_aio(fds, aiocb, WRITE_COUNT);
	printf("Test PASSED\n");
	return PTS_PASS;
}
