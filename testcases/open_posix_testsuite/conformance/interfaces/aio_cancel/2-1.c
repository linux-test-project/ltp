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
 *	if aiobcp is NULL, all outstanding cancelable AIO against fildes
 *	shall be canceled.
 *
 * method:
 *
 *	open a file and queue a write to it with aio_write()
 *	execute aio_cancel() on this file
 *	check aio_cancel() is not -1
 *	-> aio_cancel() works on fildes used with an aio command
 *
 *	we have no way to assert we generate "cancelable" AIO and thus to check
 *	if it has been really canceled
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

#define TNAME "aio_cancel/2-1.c"

int main(void)
{
	char tmpfname[256];
#define BUF_SIZE 1024
	char buf[BUF_SIZE];
	int fd, err;
	struct aiocb aiocb;

	if (sysconf(_SC_ASYNCHRONOUS_IO) < 200112L)
		return PTS_UNSUPPORTED;

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_aio_cancel_2_1_%d",
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

	switch (aio_cancel(fd, NULL)) {
	case -1:
		printf(TNAME " Error at aio_cancel(): %s\n", strerror(errno));
		return PTS_FAIL;
	case AIO_NOTCANCELED:
		do {
			struct timespec completion_wait_ts = {0, 10000000};
			nanosleep(&completion_wait_ts, NULL);
			err = aio_error(&aiocb);
		} while (err == EINPROGRESS);
	}

	close(fd);
	printf("Test PASSED\n");
	return PTS_PASS;
}
