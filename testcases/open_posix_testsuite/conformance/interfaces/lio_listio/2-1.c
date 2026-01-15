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
 *	If mode is LIO_NOWAIT, lio_listio() shall return immediately.
 *
 * method:
 *
 *	- open a socket pair
 *	- submit a list of writes to lio_listio in LIO_NOWAIT mode
 *	- check that upon return some I/Os are still running
 *	- drain the sockets
 *	- check that I/O finish signal was received
 *
 */

#include <aio.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#include "posixtest.h"
#include "tempfile.h"

#define TNAME "lio_listio/2-1.c"

#define NUM_AIOCBS	8

static int fds[2];
static struct aiocb aiocbs[NUM_AIOCBS];
static char *bufs;
static volatile int received_all;

static void sigrt2_handler(int signum PTS_ATTRIBUTE_UNUSED,
	siginfo_t *info PTS_ATTRIBUTE_UNUSED,
	void *context PTS_ATTRIBUTE_UNUSED)
{
	received_all = 1;
}

static void read_all(void)
{
	int i, ret;

	for (i = 0; i < NUM_AIOCBS; i++) {
		if (!aiocbs[i].aio_buf)
			break;

		ret = aio_error(&aiocbs[i]);

		/* flush written data from the socket */
		if (ret == 0 || ret == EINPROGRESS) {
			read(fds[1], (void *)aiocbs[i].aio_buf,
				aiocbs[i].aio_nbytes);
			aiocbs[i].aio_buf = NULL;
		}
	}
}

static void cleanup(void)
{
	read_all();
	free(bufs);
	close(fds[0]);
	close(fds[1]);
}

int main(void)
{
	struct aiocb *liocbs[NUM_AIOCBS];
	struct sigaction action;
	struct sigevent event;
	int errors = 0;
	int ret;
	int err;
	int i;
	int bufsize;
	socklen_t argsize = sizeof(bufsize);

	if (sysconf(_SC_ASYNCHRONOUS_IO) < 200112L)
		exit(PTS_UNSUPPORTED);

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
	bufs = malloc(NUM_AIOCBS * bufsize);

	if (bufs == NULL) {
		printf(TNAME " Error at malloc(): %s\n", strerror(errno));
		cleanup();
		exit(PTS_UNRESOLVED);
	}

	/* Queue up a bunch of aio writes */
	for (i = 0; i < NUM_AIOCBS; i++) {
		liocbs[i] = &aiocbs[i];
		aiocbs[i].aio_fildes = fds[0];
		aiocbs[i].aio_offset = i * bufsize;
		aiocbs[i].aio_buf = &bufs[i * bufsize];
		aiocbs[i].aio_nbytes = bufsize;
		aiocbs[i].aio_lio_opcode = LIO_WRITE;
	}

	/* Use SIGRTMIN+2 for list completion */
	event.sigev_notify = SIGEV_SIGNAL;
	event.sigev_signo = SIGRTMIN + 2;
	event.sigev_value.sival_ptr = NULL;

	/* Setup handler for list completion */
	action.sa_sigaction = sigrt2_handler;
	sigemptyset(&action.sa_mask);
	action.sa_flags = SA_SIGINFO | SA_RESTART;
	sigaction(SIGRTMIN + 2, &action, NULL);

	/* Submit request list */
	ret = lio_listio(LIO_NOWAIT, liocbs, NUM_AIOCBS, &event);

	if (ret) {
		printf(TNAME " Error at lio_listio() %d: %s\n", errno,
			strerror(errno));
		/* Clear the aiocbs or cleanup() will get stuck */
		memset(aiocbs, 0, NUM_AIOCBS * sizeof(struct aiocb));
		cleanup();
		exit(PTS_FAIL);
	}

	if (received_all != 0) {
		printf(TNAME
			" Error lio_listio() signaled completion too early\n");
		cleanup();
		exit(PTS_FAIL);
	}

	read_all();

	for (i = 0; i < 5 && !received_all; i++)
		sleep(1);

	if (received_all == 0) {
		printf(TNAME " Test did not receive I/O completion signal\n");
		cleanup();
		exit(PTS_FAIL);
	}

	/* Check return code and free things */
	for (i = 0; i < NUM_AIOCBS; i++) {
		err = aio_error(&aiocbs[i]);
		ret = aio_return(&aiocbs[i]);

		if ((err != 0) && (ret != bufsize)) {
			printf(TNAME " req %d: error = %d - return = %d\n", i,
				err, ret);
			errors++;
		}
	}

	cleanup();

	if (errors != 0)
		exit(PTS_FAIL);

	printf(TNAME " PASSED\n");
	return PTS_PASS;
}
