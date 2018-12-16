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
 *	aio_cancel() shall return AIO_CANCELED if the requested operations
 *	were canceled.
 *
 * method:
 *
 *	queue a lot of aio_write() to a given fildes.
 *	try to cancel the last one submited
 *	if aio_error() is ECANCELED and aio_cancel() is AIO_CANCELED
 *	test is passed
 *	if aio_error() is ECANCELED and aio_cancel() is NOT AIO_CANCELED
 *	test is failed
 *	if there is no aio_error() with ECANCELED and
 *	aio_cancel() is AIO_CANCELED
 *	test is failed
 *	otherwise test is unresolved
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

#define TNAME "aio_cancel/6-1.c"

#define BUF_NB		128
#define BUF_SIZE	1024

int main(void)
{
	char tmpfname[256];
	int fd;
	struct aiocb *aiocb[BUF_NB];
	int i;
	int in_progress;
	int gret;

	if (sysconf(_SC_ASYNCHRONOUS_IO) < 200112L)
		return PTS_UNSUPPORTED;

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_aio_cancel_6_1_%d",
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
		aiocb[i]->aio_fildes = fd;
		aiocb[i]->aio_buf = malloc(BUF_SIZE);
		if (aiocb[i]->aio_buf == NULL) {
			printf(TNAME " Error at malloc(): %s\n",
			       strerror(errno));
			return PTS_UNRESOLVED;
		}
		aiocb[i]->aio_nbytes = BUF_SIZE;
		aiocb[i]->aio_offset = 0;
		aiocb[i]->aio_sigevent.sigev_notify = SIGEV_NONE;

		if (aio_write(aiocb[i]) == -1) {
			printf(TNAME " loop %d: Error at aio_write(): %s\n",
			       i, strerror(errno));
			return PTS_FAIL;
		}
	}

	/* try to cancel the last one queued */

	gret = aio_cancel(fd, aiocb[i - 1]);

	if (gret == -1) {
		printf(TNAME " Error at aio_cancel(): %s\n", strerror(errno));
		return PTS_FAIL;
	}

	close(fd);

	do {
		in_progress = 0;
		for (i = 0; i < BUF_NB; i++) {
			int ret;

			ret = (aio_error(aiocb[i]));

			if (ret == -1) {
				printf(TNAME " Error at aio_error(): %s\n",
				       strerror(errno));
				return PTS_FAIL;
			} else if (ret == EINPROGRESS)
				in_progress = 1;
			else if (ret == ECANCELED) {
				if (gret == AIO_CANCELED) {
					printf("Test PASSED\n");
					return PTS_PASS;
				}

				printf(TNAME
				       " aio_cancel() is not AIO_CANCELED\n");
				return PTS_FAIL;
			}
		}
	} while (in_progress);

	if (gret == AIO_CANCELED) {
		printf(TNAME
		       " aio_cancel() is AIO_CANCELED without ECANCELED\n");
		return PTS_FAIL;
	}

	return PTS_UNRESOLVED;
}
