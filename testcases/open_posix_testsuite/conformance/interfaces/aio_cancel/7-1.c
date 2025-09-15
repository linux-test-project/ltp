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
 *	aio_cancel() shall return AIO_NOTCANCELED if at least one of the
 *	requested operations cannot be canceled because it is in progress.
 *
 * method:
 *
 *	queue multiple aio_write()s to a given socket
 *	wait for a specific task to start and block on full socket buffer
 *	then cancel all operations belonging to this socket
 *	check result of each operation:
 *	- aio_cancel() must return AIO_NOTCANCELED
 *	- aio_error() must return 0 for writes before the blocked task
 *	- aio_error() must return EINPROGRESS for the blocked task
 *	- aio_error() must return ECANCELED for writes after the blocked task
 *	If any of the above conditions is not met, fail the test.
 *	Otherwise pass.
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <aio.h>
#include <sys/socket.h>

#include "posixtest.h"

#define TNAME "aio_cancel/7-1.c"

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
	int i;
	int gret;
	int bufsize;
	int exp_ret;
	socklen_t argsize = sizeof(bufsize);
	const struct timespec sleep_ts = { .tv_sec = 0, .tv_nsec = 10000000 };

	if (sysconf(_SC_ASYNCHRONOUS_IO) < 200112L)
		return PTS_UNSUPPORTED;

	gret = socketpair(AF_UNIX, SOCK_DGRAM, 0, fds);
	if (gret == -1) {
		printf(TNAME " Error creating sockets(): %s\n",
			strerror(errno));
		return PTS_UNRESOLVED;
	}

	gret = getsockopt(fds[0], SOL_SOCKET, SO_SNDBUF, &bufsize, &argsize);
	if (gret == -1) {
		printf(TNAME " Error reading socket buffer size: %s\n",
			strerror(errno));
		cleanup();
		return PTS_UNRESOLVED;
	}

	/* Socket buffer size is twice the maximum message size */
	bufsize /= 2;

	/* create AIO req */
	for (i = 0; i < BUF_NB; i++) {
		aiocb[i].aio_fildes = fds[0];
		aiocb[i].aio_buf = malloc(bufsize);

		if (aiocb[i].aio_buf == NULL) {
			printf(TNAME " Error at malloc(): %s\n",
			       strerror(errno));
			cleanup();
			return PTS_UNRESOLVED;
		}

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
		gret = aio_error(&aiocb[BLOCKED_TASK - 1]);
		nanosleep(&sleep_ts, NULL);

		if (gret <= 0)
			break;
	}

	if (gret) {
		printf(TNAME " Task #%d failed to complete: %s\n",
			BLOCKED_TASK - 1, strerror(gret == -1 ? errno : gret));
		cleanup();
		return PTS_FAIL;
	}

	/* try to cancel all */
	gret = aio_cancel(fds[0], NULL);
	if (gret == -1) {
		printf(TNAME " Error at aio_cancel(): %s\n", strerror(errno));
		cleanup();
		return PTS_FAIL;
	}

	if (gret != AIO_NOTCANCELED) {
		printf(TNAME " Unexpected aio_cancel() return value: %s\n",
			strerror(gret));
		cleanup();
		return PTS_FAIL;
	}

	/*
	 * check write results, expected values:
	 * - 0 for the first two writes
	 * - EINPROGRESS for the third
	 * - ECANCELED for the rest
	 */
	for (i = 0, exp_ret = 0; i < BUF_NB; i++) {
		int ret = aio_error(&aiocb[i]);

		if (i == BLOCKED_TASK) {
			if (ret == EINPROGRESS) {
				exp_ret = ECANCELED;
				continue;
			}

			printf(TNAME " Bad task #%d result: %s "
				"(expected EINPROGRESS)\n",
				i, strerror(ret));
			cleanup();
			return PTS_FAIL;
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
