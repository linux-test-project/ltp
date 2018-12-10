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
 *	Error status should be set to [ECANCELED] and return shall be -1 for
 *	successfully canceled AIO.
 *
 * method:
 *
 *	we queue a lot of aio_write() operation to a file descriptor
 *	then we try to cancel all aio operation of this file descriptor
 *	we check with aio_error() state of each operation
 *	if aio_error() is ECANCELED and aio_return() is -1
 *	test is passed
 *	if aio_error() is ECANCELED and aio_return() is NOT -1
 *	test fails
 *	otherwise
 *	test is unresolved
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

#define TNAME "aio_cancel/4-1.c"

#define BUF_NB		128
#define BUF_SIZE	(1024*1024)

int main(void)
{
	char tmpfname[256];
	int fd;
	struct aiocb *aiocb[BUF_NB];
	int i;
	int in_progress;

	if (sysconf(_SC_ASYNCHRONOUS_IO) < 200112L)
		return PTS_UNSUPPORTED;

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_aio_cancel_4_1_%d",
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
		aiocb[i] = malloc(sizeof(struct aiocb));
		if (aiocb[i] == NULL) {
			printf(TNAME " Error at malloc(): %s\n",
			       strerror(errno));
			return PTS_UNRESOLVED;
		}
		memset(aiocb[i], 0, sizeof(struct aiocb));
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
	}

	for (i = 0; i < BUF_NB; i++) {
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
				if (aio_return(aiocb[i]) == -1) {
					printf("Test PASSED\n");
					return PTS_PASS;
				}

				printf(TNAME " aio_return is not -1\n");
				return PTS_FAIL;
			}
		}
	} while (in_progress);

	return PTS_UNRESOLVED;
}
