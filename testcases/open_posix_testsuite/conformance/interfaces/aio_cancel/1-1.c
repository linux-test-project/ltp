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
 *	The aiocbp argument points to the AIO control block to be canceled.
 *
 * method:
 *
 *	- create a valid aiocb with a call to aio_write()
 *	- call aio_cancel() with this aiocb and check return value is not -1
 *	-> aio_cancel() works with a valid (finished or not) aiocb
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

#define TNAME "aio_cancel/1-1.c"

int main(void)
{
	char tmpfname[256];
#define BUF_SIZE 1024
	char buf[BUF_SIZE];
	struct timespec processing_completion_ts = {0, 10000000};
	int fd, err;
	struct aiocb aiocb;

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

	switch (aio_cancel(fd, &aiocb)) {
	case -1:
		printf(TNAME " Error at aio_cancel(): %s\n", strerror(errno));
		return PTS_FAIL;
	case AIO_NOTCANCELED:
		do {
			nanosleep(&processing_completion_ts, NULL);
			err = aio_error(&aiocb);
		} while (err == EINPROGRESS);
	}

	close(fd);
	printf("Test PASSED\n");
	return PTS_PASS;
}
