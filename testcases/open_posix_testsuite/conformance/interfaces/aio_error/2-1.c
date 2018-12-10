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
 *	It the operations has not yet completed, [EINPROGRESS] shall be
 *	returned.
 *
 * method:
 *
 *	queue a lot of aio_write() to a given fildes.
 *	then check if at least on is EINPROGRESS
 *	if yes, result is pass otherwise unresolved
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

#define TNAME "aio_error/2-1.c"

#define BUF_NB		128
#define BUF_SIZE	1024

int main(void)
{
	char tmpfname[256];
	int fd;
	struct aiocb *aiocb[BUF_NB];
	int i;
	int ret;

	if (sysconf(_SC_ASYNCHRONOUS_IO) < 200112L)
		return PTS_UNSUPPORTED;

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_aio_error_2_1_%d",
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

	ret = PTS_UNRESOLVED;
	for (i = 0; i < BUF_NB; i++) {
		if (aio_error(aiocb[i]) == EINPROGRESS) {
			ret = PTS_PASS;
			break;
		}
	}

	return ret;
}
