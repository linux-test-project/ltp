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
 *	aio_return() may fail with [EINVAL]
 *	If the aiocbp does not refer to an operation whose return status
 *	has not yet been retrieved.
 *
 * method:
 *
 *	- Open a file
 *	- fill in an aiocb for writing
 *	- call aio_write usign this aiocb
 *	- fill in a new aiocb
 *	- call aio_return with this last aiocb
 */

#include <sys/stat.h>
#include <aio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "posixtest.h"

#define TNAME "aio_return/4-1.c"
#define BUF_SIZE 111

int main(void)
{
	char tmpfname[256];
	char buf[BUF_SIZE];
	struct aiocb aiocb;
	struct aiocb aiocb2;
	int fd, retval;
	struct timespec completion_wait_ts = {0, 10000000};

	if (sysconf(_SC_ASYNCHRONOUS_IO) < 200112L)
		return PTS_UNSUPPORTED;

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_aio_return_2_1_%d",
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
		close(fd);
		printf(TNAME " Error at aio_write(): %s\n", strerror(errno));
		return PTS_FAIL;
	}

	do {
		nanosleep(&completion_wait_ts, NULL);
		retval = aio_error(&aiocb);
	} while (retval == EINPROGRESS);

	retval = aio_error(&aiocb);

	if (retval == 0) {

		memset(&aiocb2, 0, sizeof(struct aiocb));
		aiocb2.aio_fildes = fd;
		aiocb2.aio_buf = buf;
		aiocb2.aio_nbytes = BUF_SIZE;

		retval = aio_return(&aiocb2);

		if (retval != -1 || aio_error(&aiocb) != EINVAL) {
			close(fd);
			printf(TNAME "aio_return() have not failed\n");
			return PTS_UNTESTED;
		}

		retval = aio_return(&aiocb);

		if (retval != BUF_SIZE) {
			close(fd);
			printf(TNAME " Error at aio_return(): %d, %s\n", retval,
			       strerror(aio_error(&aiocb)));
			return PTS_UNRESOLVED;
		}

	} else {
		close(fd);
		printf(TNAME " Error at aio_error(): %s\n", strerror(retval));
		return PTS_UNRESOLVED;
	}

	close(fd);
	printf("Test PASSED\n");
	return PTS_PASS;
}
