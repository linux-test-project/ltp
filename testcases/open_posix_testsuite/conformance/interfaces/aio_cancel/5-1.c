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
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <aio.h>

#include "posixtest.h"

#define NO_FINISHED -2
#define NO_CANCELED -3

#define TNAME "aio_cancel/5-1.c"

#define BUF_NB		128
#define BUF_SIZE	1024
#define MAX_RETRIES 50

static char aio_buffer[BUF_NB][BUF_SIZE];
static struct aiocb aiocb[BUF_NB];

int do_test(void)
{
	char tmpfname[256];
	int fd;
	int i;
	int in_progress;
	int check_one_done;
	int check_one_canceled;

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_aio_cancel_5_1_%d",
		 getpid());
	unlink(tmpfname);
	fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf(TNAME " Error at open(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}

	unlink(tmpfname);

	/* create AIO req */
	for (i = 0; i < BUF_NB; i++) {
		memset(&aiocb[i], 0, sizeof(aiocb[i]));
		aiocb[i].aio_fildes = fd;
		aiocb[i].aio_buf = aio_buffer[i];
		aiocb[i].aio_nbytes = BUF_SIZE;
		aiocb[i].aio_offset = 0;
		aiocb[i].aio_sigevent.sigev_notify = SIGEV_NONE;

		if (aio_write(&aiocb[i]) == -1) {
			printf(TNAME " loop %d: Error at aio_write(): %s\n",
			       i, strerror(errno));
			return PTS_FAIL;
		}
	}

	/* try to cancel all
	 * we hope to have enough time to cancel at least one */
	if (aio_cancel(fd, NULL) == -1) {
		printf(TNAME " Error at aio_cancel(): %s\n", strerror(errno));
		return PTS_FAIL;
	}

	close(fd);

	check_one_done = 0;
	check_one_canceled = 0;
	do {
		in_progress = 0;
		for (i = 0; i < BUF_NB; i++) {
			int ret;

			ret = (aio_error(&aiocb[i]));

			if (ret == -1) {
				printf(TNAME " Error at aio_error(): %s\n", strerror(errno));
				return PTS_FAIL;
			} else if (ret == ECANCELED) {
				check_one_canceled = 1;
			} else if ((ret == EINPROGRESS) || (ret == 0)) {
				if (ret == EINPROGRESS)
					in_progress = 1;

				check_one_done = 1;

				/* check iocb is not modified */
				if ((aiocb[i].aio_fildes != fd) ||
				    (aiocb[i].aio_buf != aio_buffer[i]) ||
				    (aiocb[i].aio_nbytes != BUF_SIZE) ||
				    (aiocb[i].aio_offset != 0) ||
				    (aiocb[i].aio_sigevent.sigev_notify != SIGEV_NONE)) {
					printf(TNAME " aiocbp modified\n");
					return PTS_FAIL;
				}
			}
		}
	} while (in_progress);

	if (!check_one_done) {
		return NO_FINISHED;
	} else if (!check_one_canceled) {
		return NO_CANCELED;
	}

	return PTS_PASS;
}

int main(void)
{
	int num_retries;

	if (sysconf(_SC_ASYNCHRONOUS_IO) < 200112L)
		return PTS_UNSUPPORTED;

	for (num_retries = 0; num_retries < MAX_RETRIES; ++num_retries) {
		int ret = do_test();

		if (ret == NO_FINISHED || ret == NO_CANCELED) {
			printf(TNAME " INFO retry (no requests %s)\n", 
				   ret == NO_FINISHED ? "finished" : " canceled");
			continue;
		}

		if (ret == PTS_PASS)
			printf(TNAME " PASS (%d retries)\n", num_retries);

		return ret;
	}

	printf(TNAME " UNRESOLVED: In %d retries not one request finished and while one was canceled\n",
		   MAX_RETRIES);

	return PTS_UNRESOLVED;
}
