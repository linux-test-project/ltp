/*
 * Copyright (c) 2004, Bull SA. All rights reserved.
 *  Created by:  Laurent.Vivier@bull.net
 * Copyright (c) 2013 Cyril Hrubis <chrubis@suse.cz>
 *
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 *
 *	aio_suspend() shall fail if:
 *	[EAGAIN] No AIO indicated in the list completed before timeout
 *
 * method:
 *
 *	- write to a file
 *	- submit a list of read requests
 *	- check that the selected request has not completed
 *	- suspend on selected request
 *	- check that the suspend timed out and returned EAGAIN
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

#define WAIT_FOR_AIOCB	6

static sig_atomic_t received_all;

static void sigrt1_handler()
{
	received_all = 1;
}

static int do_test(int num_aiocbs, size_t buf_size)
{
	struct timespec processing_completion_ts = {0, 10000000};
	char tmpfname[256];
	int fd;
	struct aiocb *aiocbs[num_aiocbs];
	struct aiocb *plist[2];
	char *bufs;
	struct sigaction action;
	struct sigevent event;
	struct timespec ts = {0, 1000000};	/* 1 ms */
	int ret, ret2;
	int err = PTS_UNRESOLVED;
	int i;

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_aio_suspend_9_1_%d",
		 getpid());
	unlink(tmpfname);

	fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);

	if (fd == -1) {
		printf("Error at open(): %s\n", strerror(errno));
		goto err0;
	}

	unlink(tmpfname);

	int file_size = num_aiocbs * buf_size;

	bufs = malloc(file_size);

	if (bufs == NULL) {
		printf("Error at malloc(): %s\n", strerror(errno));
		goto err1;
	}

	ret = write(fd, bufs, file_size);
	if (ret != file_size) {

		if (ret < 0)
			printf("Error at write(): %s\n", strerror(errno));
		else
			printf("Error at write(): %i of %i written\n",
			       ret, file_size);

		goto err2;
	}

	/* Queue up a bunch of aio reads */
	for (i = 0; i < num_aiocbs; i++) {
		aiocbs[i] = malloc(sizeof(struct aiocb));
		memset(aiocbs[i], 0, sizeof(struct aiocb));

		aiocbs[i]->aio_fildes = fd;
		aiocbs[i]->aio_offset = i * buf_size;
		aiocbs[i]->aio_buf = &bufs[i * buf_size];
		aiocbs[i]->aio_nbytes = buf_size;
		aiocbs[i]->aio_lio_opcode = LIO_READ;
	}

	/* reset the completion flag */
	received_all = 0;

	/* Use SIGRTMIN+1 for list completion */
	event.sigev_notify = SIGEV_SIGNAL;
	event.sigev_signo = SIGRTMIN + 1;
	event.sigev_value.sival_ptr = NULL;

	action.sa_sigaction = sigrt1_handler;
	sigemptyset(&action.sa_mask);
	action.sa_flags = SA_SIGINFO | SA_RESTART;
	sigaction(SIGRTMIN + 1, &action, NULL);

	/* Setup suspend list */
	plist[0] = NULL;
	plist[1] = aiocbs[WAIT_FOR_AIOCB];

	/* Submit request list */
	ret = lio_listio(LIO_NOWAIT, aiocbs, num_aiocbs, &event);

	if (ret) {
		printf(" Error at lio_listio() %d: %s\n",
		       errno, strerror(errno));
		goto err3;
	}

	/* Suspend on selected request */
	ret = aio_suspend((const struct aiocb **)plist, 2, &ts);

	/* Check selected request has not completed yet */
	ret2 = aio_error(aiocbs[WAIT_FOR_AIOCB]);
	if (ret2 != EINPROGRESS) {
		/*
		 * The operation was too fast, wait for completion
		 * and redo it with larger buffers.
		 */
		err = -1;
		goto err4;
	}

	/* timed out aio_suspend should return -1 and set errno to EAGAIN */
	if (ret != -1) {
		printf("aio_suspend() should return -1\n");
		err = PTS_FAIL;
		goto err4;
	}
	if (errno != EAGAIN) {
		printf("aio_suspend() should set errno to EAGAIN: %d (%s)\n",
		       errno, strerror(errno));
		err = PTS_FAIL;
		goto err4;
	}

	/* Wait for list processing completion */
	while (!received_all)
		nanosleep(&processing_completion_ts, NULL);

	/* Check return values and errors */
	err = PTS_PASS;

	for (i = 0; i < num_aiocbs; i++) {
		err = aio_error(aiocbs[i]);
		ret = aio_return(aiocbs[i]);

		if ((err != 0) && ((size_t)ret != buf_size)) {
			printf("req %d: error = %d - return = %d\n",
			       i, err, ret);
			err = PTS_FAIL;
		}
	}

err4:
	while (!received_all)
		nanosleep(&processing_completion_ts, NULL);
err3:
	for (i = 0; i < num_aiocbs; i++)
		free(aiocbs[i]);
err2:
	free(bufs);
err1:
	close(fd);
err0:
	return err;
}

int main(void)
{
	int aio_cbs = 10;
	int buf_size = 1024 * 64;
	int ret;

	if (sysconf(_SC_ASYNCHRONOUS_IO) < 200112L)
		return PTS_UNSUPPORTED;

	/* Repeat the test with increasing buffer size */
	do {
		ret = do_test(aio_cbs, buf_size);
		buf_size += buf_size / 4;
	} while (ret == -1);

	if (ret != 0)
		return ret;

	printf("(buf_size = %i)\nTest PASSED\n", buf_size);
	return PTS_PASS;
}
