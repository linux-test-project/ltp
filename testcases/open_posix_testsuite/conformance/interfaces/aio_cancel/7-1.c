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
 *	aio_cancel() shall return AIO_NOTCANCELED if at least one of the
 *	requested operations cannot be canceled because it is in progress.
 *
 * method:
 *
 *	queue a lot of aio_write() to a given file descriptor
 *	then cancel all operation belonging to this file descriptor
 *	check result of each operation:
 *	- if aio_error() is EINPROGRESS and aio_cancel() is not AIO_NOTCANCELED
 *	  result is failed
 *	- if aio_error() is succes (0) and aio_cancel() is AIO_NOTCANCELED
 *	  result is susccess
 *	- otherwise result is unresolved
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

#define TNAME "aio_cancel/7-1.c"

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

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_aio_cancel_7_1_%d",
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
			close(fd);
			return PTS_UNRESOLVED;
		}

		aiocb[i]->aio_fildes = fd;
		aiocb[i]->aio_buf = malloc(BUF_SIZE);

		if (aiocb[i]->aio_buf == NULL) {
			printf(TNAME " Error at malloc(): %s\n",
			       strerror(errno));
			close(fd);
			return PTS_UNRESOLVED;
		}

		aiocb[i]->aio_nbytes = BUF_SIZE;
		aiocb[i]->aio_offset = 0;
		aiocb[i]->aio_sigevent.sigev_notify = SIGEV_NONE;

		if (aio_write(aiocb[i]) == -1) {
			printf(TNAME " loop %d: Error at aio_write(): %s\n",
			       i, strerror(errno));
			close(fd);
			return PTS_FAIL;
		}
	}

	/* try to cancel all
	 * we hope to have enough time to cancel at least one
	 */
	gret = aio_cancel(fd, NULL);
	if (gret == -1) {
		printf(TNAME " Error at aio_cancel(): %s\n", strerror(errno));
		close(fd);
		return PTS_FAIL;
	}

	do {
		in_progress = 0;
		for (i = 0; i < BUF_NB; i++) {
			int ret = aio_error(aiocb[i]);

			switch (ret) {
			case -1:
				printf(TNAME " Error at aio_error(): %s\n",
				       strerror(errno));
				close(fd);
				return PTS_FAIL;
				break;
			case EINPROGRESS:
				/* at this point, all operations should be:
				 *    canceled
				 * or in progress
				 *    with aio_cancel() == AIO_NOTCANCELED
				 */
				if (gret != AIO_NOTCANCELED) {
					printf(TNAME
					       " Error at aio_error(): %s\n",
					       strerror(errno));
					close(fd);
					return PTS_FAIL;
				}

				in_progress = 1;
				break;
			case 0:
				/* we seek one not canceled and check why.
				 * (perhaps) it has not been canceled
				 * because it was in progress
				 * during the cancel operation
				 */
				if (gret == AIO_NOTCANCELED) {
					printf("Test PASSED\n");
					close(fd);
					return PTS_PASS;
				}
				break;
			}
		}
	} while (in_progress);

	close(fd);

	return PTS_UNRESOLVED;
}
