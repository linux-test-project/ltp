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

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <aio.h>
#include <sys/socket.h>

#include "posixtest.h"

#define TNAME "aio_cancel/5-1.c"

#define BUF_NB		8
#define BLOCKED_TASK	2

static int fds[2];
static struct aiocb aiocb[BUF_NB];

static void cleanup(void)
{
	int i, ret;

	for (i = 0; i < BUF_NB; i++) {
		if (!aiocb[i].aio_buf)
			break;

		ret = aio_error(&aiocb[i]);

		/* flush written data from the socket */
		if (ret == 0 || ret == EINPROGRESS) {
			read(fds[1], (void *)aiocb[i].aio_buf,
				aiocb[i].aio_nbytes);
		}

		free((void *)aiocb[i].aio_buf);
	}

	close(fds[0]);
	close(fds[1]);
}

int main(void)
{
	char *buf[BUF_NB];
	int i;
	int ret;
	int bufsize;
	int exp_ret;
	socklen_t argsize = sizeof(bufsize);
	const struct timespec sleep_ts = { .tv_sec = 0, .tv_nsec = 10000000 };

	if (sysconf(_SC_ASYNCHRONOUS_IO) < 200112L)
		return PTS_UNSUPPORTED;

	ret = socketpair(AF_UNIX, SOCK_DGRAM, 0, fds);
	if (ret == -1) {
		printf(TNAME " Error creating sockets(): %s\n",
			strerror(errno));
		return PTS_UNRESOLVED;
	}

	ret = getsockopt(fds[0], SOL_SOCKET, SO_SNDBUF, &bufsize, &argsize);
	if (ret == -1) {
		printf(TNAME " Error reading socket buffer size: %s\n",
			strerror(errno));
		cleanup();
		return PTS_UNRESOLVED;
	}

	/* Socket buffer size is twice the maximum message size */
	bufsize /= 2;

	/* create AIO req */
	for (i = 0; i < BUF_NB; i++) {
		buf[i] = malloc(bufsize);
		if (buf[i] == NULL) {
			printf(TNAME " Error at malloc(): %s\n",
				strerror(errno));
			cleanup();
			return PTS_UNRESOLVED;
		}
		aiocb[i].aio_fildes = fds[0];
		aiocb[i].aio_buf = buf[i];
		aiocb[i].aio_nbytes = bufsize;
		aiocb[i].aio_offset = 0;
		aiocb[i].aio_sigevent.sigev_notify = SIGEV_NONE;

		if (aio_write(&aiocb[i]) == -1) {
			printf(TNAME " loop %d: Error at aio_write(): %s\n",
				i, strerror(errno));
			cleanup();
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
		cleanup();
		return PTS_FAIL;
	}

	/* try to cancel all */
	ret = aio_cancel(fds[0], NULL);
	if (ret == -1) {
		printf(TNAME " Error at aio_cancel(): %s\n", strerror(errno));
		cleanup();
		return PTS_FAIL;
	}

	if (ret != AIO_NOTCANCELED) {
		printf(TNAME " Unexpected aio_cancel() return value: %s\n",
			strerror(ret));
		cleanup();
		return PTS_FAIL;
	}

	for (i = 0, exp_ret = 0; i < BUF_NB; i++) {
		ret = (aio_error(&aiocb[i]));

		if (i == BLOCKED_TASK) {
			if (ret != EINPROGRESS) {
				printf(TNAME " Bad task #%d result: %s "
					"(expected EINPROGRESS)\n",
					i, strerror(ret));
				cleanup();
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
				cleanup();
				return PTS_FAIL;
			}

			exp_ret = ECANCELED;
			continue;
		}

		if (ret != exp_ret) {
			printf(TNAME " Bad task #%d result: %s (expected %s)\n",
				i, strerror(ret), strerror(exp_ret));
			cleanup();
			return PTS_FAIL;
		}
	}

	cleanup();
	printf("Test PASSED\n");
	return PTS_PASS;
}
