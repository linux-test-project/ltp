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
 *	On a timeout exit, aio_suspend shall return with an error.
 *
 * method: Testing for a non NULL timeout
 *
 *	- submit a list of write requests
 *	- check that the selected request has not completed
 *	- suspend on selected request
 *	- check that the suspend timed out
 *	- check return code and errno
 *
 */

#include <sys/stat.h>
#include <aio.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "posixtest.h"

#define TNAME "aio_suspend/4-1.c"

#define NUM_AIOCBS	10
#define BUF_SIZE	(2 * 1024 * 1024)
#define WAIT_FOR_AIOCB	6
#define WAIT_FOR_AIOCB_BUF_SIZE (20 * 1024 * 1024)

static volatile int received_selected;
static volatile int received_all;

static void sigrt1_handler(int signum)
{
	(void)signum;

	received_selected = 1;
}

static void sigrt2_handler(int signum)
{
	(void)signum;

	received_all = 1;
}

int main(void)
{
	char tmpfname[256];
	struct aiocb aiocbs[NUM_AIOCBS];
	struct aiocb *aiolist[NUM_AIOCBS];
	struct aiocb *plist[2];
	struct sigaction action;
	struct sigevent event;
	struct timespec ts = {0, 10};
	int errors = 0;
	int ret, err, i, rval, fd;
	struct timespec processing_completion_ts = {0, 50000000};

	if (sysconf(_SC_ASYNCHRONOUS_IO) < 200112L)
		return PTS_UNSUPPORTED;

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_aio_suspend_4_1_%d",
		 getpid());
	unlink(tmpfname);

	fd = open(tmpfname, O_SYNC | O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);

	if (fd == -1) {
		printf(TNAME " Error at open(): %s\n", strerror(errno));
		exit(PTS_UNRESOLVED);
	}

	unlink(tmpfname);

	memset(aiocbs, 0, sizeof(aiocbs));
	size_t buf_offset = 0;

	/* Queue up a bunch of aio reads */
	for (i = 0; i < NUM_AIOCBS; i++) {
		size_t buf_size = (i == WAIT_FOR_AIOCB) ?
		                   WAIT_FOR_AIOCB_BUF_SIZE : BUF_SIZE;

		aiocbs[i].aio_fildes = fd;
		aiocbs[i].aio_offset = buf_offset;
		aiocbs[i].aio_buf = malloc(buf_size);
		aiocbs[i].aio_nbytes = buf_size;
		aiocbs[i].aio_lio_opcode = LIO_WRITE;

		if (!aiocbs[i].aio_buf) {
			perror("malloc()");
			rval = PTS_UNRESOLVED;
			goto exit;
		}

		/* Use SIGRTMIN+1 for individual completions */
		if (i == WAIT_FOR_AIOCB) {
			aiocbs[i].aio_sigevent.sigev_notify = SIGEV_SIGNAL;
			aiocbs[i].aio_sigevent.sigev_signo = SIGRTMIN + 1;
			aiocbs[i].aio_sigevent.sigev_value.sival_int = i;
		}

		buf_offset += buf_size;
		aiolist[i] = &aiocbs[i];
	}

	/* Use SIGRTMIN+2 for list completion */
	event.sigev_notify = SIGEV_SIGNAL;
	event.sigev_signo = SIGRTMIN + 2;
	event.sigev_value.sival_ptr = NULL;

	/* Setup handler for individual operation completion */
	action.sa_handler = sigrt1_handler;
	sigemptyset(&action.sa_mask);
	action.sa_flags = SA_SIGINFO | SA_RESTART;
	sigaction(SIGRTMIN + 1, &action, NULL);

	/* Setup handler for list completion */
	action.sa_handler = sigrt2_handler;
	sigemptyset(&action.sa_mask);
	action.sa_flags = SA_SIGINFO | SA_RESTART;
	sigaction(SIGRTMIN + 2, &action, NULL);

	/* Setup suspend list */
	plist[0] = NULL;
	plist[1] = &aiocbs[WAIT_FOR_AIOCB];

	/* Submit request list */
	ret = lio_listio(LIO_NOWAIT, aiolist, NUM_AIOCBS, &event);

	if (ret) {
		printf(TNAME " Error at lio_listio() %d: %s\n",
		       errno, strerror(errno));
		rval = PTS_UNRESOLVED;
		goto exit;
	}

	/* Check selected request has not completed yet */
	if (received_selected) {
		printf(TNAME " Error : AIOCB %d already completed"
		       " before suspend\n", WAIT_FOR_AIOCB);
		rval = PTS_FAIL;
		goto exit;
	}

	/* Suspend on selected request */
	ret = aio_suspend((const struct aiocb **)plist, 2, &ts);

	/* Check selected request has not completed */
	if (received_selected) {
		printf(TNAME " Error : AIOCB %d should not have completed"
		       " after timed out suspend\n", WAIT_FOR_AIOCB);
		rval = PTS_FAIL;
		goto exit;
	}

	/* timed out aio_suspend should return -1 and set errno to EAGAIN */
	if (ret != -1) {
		printf(TNAME " aio_suspend() should return -1\n");
		rval = PTS_FAIL;
		goto exit;
	}

	if (errno != EAGAIN) {
		printf(TNAME " aio_suspend() should set errno to EAGAIN:"
		       " %d (%s)\n", errno, strerror(errno));
		rval = PTS_FAIL;
		goto exit;
	}

	int retries = 6000;

	/* Wait for list processing completion */
	while (!received_all && retries-- > 0)
		nanosleep(&processing_completion_ts, NULL);

	if (retries <= 0) {
		printf(TNAME " timeouted while waiting for I/O completion");
		rval = PTS_UNRESOLVED;
		goto exit;
	}

	for (i = 0; i < NUM_AIOCBS; i++) {
		err = aio_error(&aiocbs[i]);
		ret = aio_return(&aiocbs[i]);

		if ((err != 0) && (ret != BUF_SIZE)) {
			printf(TNAME " req %d: error = %d - return = %d\n",
			       i, err, ret);
			errors++;
		}
	}

	if (errors == 0) {
		printf(TNAME " PASSED\n");
		rval = PTS_PASS;
	} else {
		rval = PTS_FAIL;
	}

exit:
	for (i = 0; i < NUM_AIOCBS; i++)
		free((void*)aiocbs[i].aio_buf);
	close(fd);
	return rval;
}
