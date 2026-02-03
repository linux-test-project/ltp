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
 *	iocbp not successfully canceled shall not be modified
 *
 * method:
 *
 *	queue some aio_write() to a file descriptor
 *	cancel all operations for this file descriptor
 *	for all operations not canceled at end of operations
 *	verify that values in aiocb is the good ones
 *	if all operations are canceled, result is unresolved
 *	if some operation not canceled are modified, test fails
 *	if all operations not canceled (at least one) are good
 *	result is pass
 *
 */

#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>

#include "posixtest.h"
#include "aio_test.h"

#define TNAME "aio_cancel/5-1.c"

#define BUF_NB		8
#define BLOCKED_TASK	2

static int fds[2];
static struct aiocb aiocb[BUF_NB];

int main(void)
{
	char *buf[BUF_NB];
	int i;
	int ret;
	int bufsize;
	int exp_ret;
	const struct timespec sleep_ts = { .tv_sec = 0, .tv_nsec = 10000000 };

	if (sysconf(_SC_ASYNCHRONOUS_IO) < 200112L)
		return PTS_UNSUPPORTED;

	if (setup_aio(TNAME, fds, aiocb, BUF_NB))
		return PTS_UNRESOLVED;

	bufsize = aiocb[0].aio_nbytes;

	/* submit AIO req */
	for (i = 0; i < BUF_NB; i++) {
		buf[i] = (char *)aiocb[i].aio_buf;

		if (aio_write(&aiocb[i]) == -1) {
			printf(TNAME " loop %d: Error at aio_write(): %s\n",
				i, strerror(errno));
			cleanup_aio(fds, aiocb, BUF_NB);
			return PTS_FAIL;
		}
	}

	/* wait for write #2 to start and get blocked by full socket buffer */
	for (i = 0; i < 1000; i++) {
		ret = aio_error(&aiocb[BLOCKED_TASK - 1]);
		nanosleep(&sleep_ts, NULL);

		if (ret <= 0)
			break;
	}

	if (ret) {
		printf(TNAME " Task #%d failed to complete: %s\n",
			BLOCKED_TASK - 1, strerror(ret == -1 ? errno : ret));
		cleanup_aio(fds, aiocb, BUF_NB);
		return PTS_FAIL;
	}

	/* try to cancel all */
	ret = aio_cancel(fds[0], NULL);
	if (ret == -1) {
		printf(TNAME " Error at aio_cancel(): %s\n", strerror(errno));
		cleanup_aio(fds, aiocb, BUF_NB);
		return PTS_FAIL;
	}

	if (ret != AIO_NOTCANCELED) {
		printf(TNAME " Unexpected aio_cancel() return value: %s\n",
			strerror(ret));
		cleanup_aio(fds, aiocb, BUF_NB);
		return PTS_FAIL;
	}

	for (i = 0, exp_ret = 0; i < BUF_NB; i++) {
		ret = (aio_error(&aiocb[i]));

		if (i == BLOCKED_TASK) {
			if (ret != EINPROGRESS) {
				printf(TNAME " Bad task #%d result: %s "
					"(expected EINPROGRESS)\n",
					i, strerror(ret));
				cleanup_aio(fds, aiocb, BUF_NB);
				return PTS_FAIL;
			}

			/* check iocb is not modified */
			if ((aiocb[i].aio_fildes != fds[0]) ||
				(aiocb[i].aio_buf != buf[i]) ||
				(aiocb[i].aio_nbytes != (size_t)bufsize) ||
				(aiocb[i].aio_offset != 0) ||
				(aiocb[i].aio_sigevent.sigev_notify !=
				SIGEV_NONE)) {

				printf(TNAME " aiocbp modified\n");
				cleanup_aio(fds, aiocb, BUF_NB);
				return PTS_FAIL;
			}

			exp_ret = ECANCELED;
			continue;
		}

		if (ret != exp_ret) {
			printf(TNAME " Bad task #%d result: %s (expected %s)\n",
				i, strerror(ret), strerror(exp_ret));
			cleanup_aio(fds, aiocb, BUF_NB);
			return PTS_FAIL;
		}
	}

	cleanup_aio(fds, aiocb, BUF_NB);
	printf("Test PASSED\n");
	return PTS_PASS;
}
