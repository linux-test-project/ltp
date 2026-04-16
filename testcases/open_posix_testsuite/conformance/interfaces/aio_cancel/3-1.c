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
 *	Asynchronous notification shall occur for AIO that are successfully
 *	cancelled.
 *
 * method:
 *
 *	open a pair of sockets and queue writes to them with aio_write()
 *	execute aio_cancel() on the socket
 *	then analyze aio_error() in the event handler
 *	if number of sig events is not equal to number of write requests,
 *	the test fails
 *	if aio_error() returns ECANCELED for the expected requests,
 *	the test passes
 *
 */

#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>

#include "posixtest.h"
#include "aio_test.h"

#define TNAME "aio_cancel/3-1.c"

#define WRITE_COUNT	8
#define MAX_COMPLETE	3
#define MAX_WAIT_RETRIES 100

static volatile int countdown = WRITE_COUNT;
static volatile int canceled;
static int fds[2];
static struct aiocb aiocb[WRITE_COUNT];

static void sig_handler(int signum PTS_ATTRIBUTE_UNUSED, siginfo_t *info,
	void *context PTS_ATTRIBUTE_UNUSED)
{
	struct aiocb *a = info->si_value.sival_ptr;

	if (aio_error(a) == ECANCELED)
		canceled++;

	aio_return(a);		/* free entry */
	countdown--;
}

int test_main(int argc PTS_ATTRIBUTE_UNUSED, char **argv PTS_ATTRIBUTE_UNUSED)
{
	struct sigaction action;
	struct timespec processing_completion_ts = {0, 10000000};
	int i;

	if (sysconf(_SC_ASYNCHRONOUS_IO) < 200112L) {
		printf(TNAME " Unsupported AIO version: %ld\n",
			sysconf(_SC_ASYNCHRONOUS_IO));
		return PTS_UNSUPPORTED;
	}

	/* install signal handler */
	action.sa_sigaction = sig_handler;
	sigemptyset(&action.sa_mask);
	action.sa_flags = SA_SIGINFO | SA_RESTART;

	if (sigaction(SIGRTMIN + 1, &action, NULL)) {
		printf(TNAME " Error at sigaction(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}

	if (setup_aio(TNAME, fds, aiocb, WRITE_COUNT))
		return PTS_UNRESOLVED;

	/* submit AIO req */
	for (i = 0; i < WRITE_COUNT; i++) {
		aiocb[i].aio_sigevent.sigev_notify = SIGEV_SIGNAL;
		aiocb[i].aio_sigevent.sigev_signo = SIGRTMIN + 1;
		aiocb[i].aio_sigevent.sigev_value.sival_ptr = &aiocb[i];
		aiocb[i].aio_reqprio = 0;

		if (aio_write(&aiocb[i]) == -1) {
			printf(TNAME " loop %d: Error at aio_write(): %s\n",
				i, strerror(errno));
			cleanup_aio(fds, aiocb, WRITE_COUNT);
			return PTS_FAIL;
		}
	}

	/* cancel all requests */
	if (aio_cancel(fds[0], NULL) == -1) {
		printf(TNAME " Error at aio_cancel(): %s\n", strerror(errno));
		cleanup_aio(fds, aiocb, WRITE_COUNT);
		return PTS_FAIL;
	}

	cleanup_aio(fds, aiocb, WRITE_COUNT);

	/* wait for signal delivery */
	for (i = 0; countdown && i < MAX_WAIT_RETRIES; i++)
		nanosleep(&processing_completion_ts, NULL);

	if (countdown) {
		printf(TNAME " %d task completion signals were not delivered",
			countdown);
		return PTS_FAIL;
	}

	if (canceled < WRITE_COUNT - MAX_COMPLETE) {
		printf(TNAME " %d AIO requests got canceled, expected %d",
			canceled, WRITE_COUNT - MAX_COMPLETE);
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
