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
 *	aio_cancel() shall return AIO_ALLDONE if all operation have already
 *	completed
 *
 * method:
 *
 *	execute one aio_write(), wait it is finished
 *	execute aio_cancel
 *	if result is AIO_ALLDONE, test is passed
 *	otherwise it is failed
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
#include <time.h>

#include "posixtest.h"

#define TNAME "aio_cancel/8-1.c"

int main(void)
{
	char tmpfname[256];
#define BUF_SIZE 1024
	char buf[BUF_SIZE];
	int fd;
	int ret;
	struct aiocb aiocb;
	struct timespec processing_completion_ts = {0, 10000000};

	if (sysconf(_SC_ASYNCHRONOUS_IO) < 200112L)
		return PTS_UNSUPPORTED;

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_aio_cancel_1_1_%d",
		 getpid());
	unlink(tmpfname);
	fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf(TNAME " Error at open(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}

	unlink(tmpfname);

	memset(buf, 0xaa, BUF_SIZE);
	memset(&aiocb, 0, sizeof(struct aiocb));
	aiocb.aio_fildes = fd;
	aiocb.aio_buf = buf;
	aiocb.aio_nbytes = BUF_SIZE;

	if (aio_write(&aiocb) == -1) {
		printf(TNAME " Error at aio_write(): %s\n", strerror(errno));
		return PTS_FAIL;
	}

	do {
		nanosleep(&processing_completion_ts, NULL);
		ret = aio_error(&aiocb);
	} while (ret == EINPROGRESS);
	if (ret < 0) {
		printf(TNAME " Error at aio_error() : %s\n", strerror(ret));
		exit(PTS_FAIL);
	}

	if (aio_cancel(fd, &aiocb) != AIO_ALLDONE) {
		printf(TNAME " Error at aio_cancel(): %s\n", strerror(errno));
		return PTS_FAIL;
	}

	close(fd);
	printf("Test PASSED\n");
	return PTS_PASS;
}
