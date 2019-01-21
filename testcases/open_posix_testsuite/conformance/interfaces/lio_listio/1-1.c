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
 *	If mode is LIO_WAIT, lio_listio() shall wait until all I/O is
 *	complete and the sig argument shall be ignored.
 *
 * method:
 *
 *	- open a file for writing
 *	- submit a list of writes to lio_listio in LIO_WAIT mode
 *	- check that upon return all I/Os are completed
 *	- check that the sig argument is ignored
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
#include <unistd.h>
#include "posixtest.h"

#define TNAME "lio_listio/1-1.c"

#define NUM_AIOCBS	10
#define BUF_SIZE	1024*1024

static volatile int received_all = 0;

void sigrt1_handler(int signum LTP_ATTRIBUTE_UNUSED,
	siginfo_t *info LTP_ATTRIBUTE_UNUSED,
	void *context LTP_ATTRIBUTE_UNUSED)
{
	received_all = 1;
}

int main(void)
{
	char tmpfname[256];
	int fd;

	struct aiocb **aiocbs;
	char *bufs;
	struct sigaction action;
	struct sigevent event;
	int errors = 0;
	int ret;
	int err;
	int i;

	if (sysconf(_SC_ASYNCHRONOUS_IO) < 200112L)
		exit(PTS_UNSUPPORTED);

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_lio_listio_1_1_%d",
		 getpid());
	unlink(tmpfname);

	fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);

	if (fd == -1) {
		printf(TNAME " Error at open(): %s\n", strerror(errno));
		exit(PTS_UNRESOLVED);
	}

	unlink(tmpfname);

	bufs = malloc(NUM_AIOCBS * BUF_SIZE);

	if (bufs == NULL) {
		printf(TNAME " Error at malloc(): %s\n", strerror(errno));
		close(fd);
		exit(PTS_UNRESOLVED);
	}

	aiocbs = malloc(sizeof(struct aiocb *) * NUM_AIOCBS);

	/* Queue up a bunch of aio writes */
	for (i = 0; i < NUM_AIOCBS; i++) {

		aiocbs[i] = malloc(sizeof(struct aiocb));
		memset(aiocbs[i], 0, sizeof(struct aiocb));

		aiocbs[i]->aio_fildes = fd;
		aiocbs[i]->aio_offset = 0;
		aiocbs[i]->aio_buf = &bufs[i * BUF_SIZE];
		aiocbs[i]->aio_nbytes = BUF_SIZE;
		aiocbs[i]->aio_lio_opcode = LIO_WRITE;

	}

	/* Use SIGRTMIN+1 for list completion */
	event.sigev_notify = SIGEV_SIGNAL;
	event.sigev_signo = SIGRTMIN + 1;
	event.sigev_value.sival_ptr = NULL;

	/* Setup handler for list completion */
	action.sa_sigaction = sigrt1_handler;
	sigemptyset(&action.sa_mask);
	action.sa_flags = SA_SIGINFO | SA_RESTART;
	sigaction(SIGRTMIN + 1, &action, NULL);

	/* Submit request list */
	ret = lio_listio(LIO_WAIT, aiocbs, NUM_AIOCBS, &event);

	if (ret) {
		printf(TNAME " Error at lio_listio() %d: %s\n", errno,
		       strerror(errno));
		for (i = 0; i < NUM_AIOCBS; i++)
			free(aiocbs[i]);
		free(bufs);
		free(aiocbs);
		close(fd);
		exit(PTS_FAIL);
	}

	if (received_all != 0) {
		printf(TNAME " lio_listio() did not ignore the sig argument\n");
		for (i = 0; i < NUM_AIOCBS; i++)
			free(aiocbs[i]);
		free(bufs);
		free(aiocbs);
		close(fd);
		exit(PTS_FAIL);
	}

	/* Check return code and free things */
	for (i = 0; i < NUM_AIOCBS; i++) {
		err = aio_error(aiocbs[i]);
		ret = aio_return(aiocbs[i]);

		if ((err != 0) && (ret != BUF_SIZE)) {
			printf(TNAME " req %d: error = %d - return = %d\n", i,
			       err, ret);
			errors++;
		}

		free(aiocbs[i]);
	}

	free(bufs);
	free(aiocbs);

	close(fd);

	if (errors != 0)
		exit(PTS_FAIL);

	printf(TNAME " PASSED\n");

	return PTS_PASS;
}
