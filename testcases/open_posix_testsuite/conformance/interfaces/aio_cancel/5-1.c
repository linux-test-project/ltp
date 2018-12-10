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
 *	queue a some aio_write() to a file descriptor
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

#define TNAME "aio_cancel/5-1.c"

#define BUF_NB		128
#define BUF_SIZE	1024

int main(void)
{
	char tmpfname[256];
	int fd;
	struct aiocb *aiocb[BUF_NB];
	char *buf[BUF_NB];
	int i;
	int in_progress;
	static int check_one;

	if (sysconf(_SC_ASYNCHRONOUS_IO) < 200112L)
		return PTS_UNSUPPORTED;

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
		aiocb[i] = calloc(1, sizeof(struct aiocb));
		if (aiocb[i] == NULL) {
			printf(TNAME " Error at malloc(): %s\n",
			       strerror(errno));
			return PTS_UNRESOLVED;
		}
		buf[i] = malloc(BUF_SIZE);
		if (buf[i] == NULL) {
			printf(TNAME " Error at malloc(): %s\n",
			       strerror(errno));
			return PTS_UNRESOLVED;
		}
		aiocb[i]->aio_fildes = fd;
		aiocb[i]->aio_buf = buf[i];
		aiocb[i]->aio_nbytes = BUF_SIZE;
		aiocb[i]->aio_offset = 0;
		aiocb[i]->aio_sigevent.sigev_notify = SIGEV_NONE;

		if (aio_write(aiocb[i]) == -1) {
			printf(TNAME " loop %d: Error at aio_write(): %s\n",
			       i, strerror(errno));
			return PTS_FAIL;
		}
	}

	/* try to cancel all
	 * we hope to have enough time to cancel at least one
	 */

	if (aio_cancel(fd, NULL) == -1) {
		printf(TNAME " Error at aio_cancel(): %s\n", strerror(errno));
		return PTS_FAIL;
	}

	close(fd);

	check_one = 0;
	do {
		in_progress = 0;
		for (i = 0; i < BUF_NB; i++) {
			int ret;

			ret = (aio_error(aiocb[i]));

			if (ret == -1) {
				printf(TNAME " Error at aio_error(): %s\n",
				       strerror(errno));
				return PTS_FAIL;
			} else if ((ret == EINPROGRESS) || (ret == 0)) {
				if (ret == EINPROGRESS)
					in_progress = 1;

				check_one = 1;

				/* check iocb is not modified */

				if ((aiocb[i]->aio_fildes != fd) ||
				    (aiocb[i]->aio_buf != buf[i]) ||
				    (aiocb[i]->aio_nbytes != BUF_SIZE) ||
				    (aiocb[i]->aio_offset != 0) ||
				    (aiocb[i]->aio_sigevent.sigev_notify !=
				     SIGEV_NONE)) {
					printf(TNAME " aiocbp modified\n");
					return PTS_FAIL;
				}
			}
		}
	} while (in_progress);

	if (!check_one)
		return PTS_UNRESOLVED;

	return PTS_PASS;
}
