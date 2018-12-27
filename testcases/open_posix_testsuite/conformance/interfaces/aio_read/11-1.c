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
 * aio_read() shall fail with [EINVAL] or the error status of the operation
 * shall be [EINVAL] if aio_offset would be invalid, or aio_reqprio is not
 * a valid value, or aio_nbytes is an invalid value.
 *
 * Testing invalid offset
 *
 * method:
 *
 *	- Create an aiocb with an invalid aio_offset
 *	- call aio_read with this aiocb
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

#define TNAME "aio_read/11-1.c"

int main(void)
{
	char tmpfname[256];
#define BUF_SIZE 111
	char buf[BUF_SIZE];
	int fd;
	struct aiocb aiocb;

	if (sysconf(_SC_ASYNCHRONOUS_IO) < 200112L)
		return PTS_UNSUPPORTED;

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_aio_read_11_1_%d",
		 getpid());
	unlink(tmpfname);
	fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf(TNAME " Error at open(): %s\n", strerror(errno));
		exit(PTS_UNRESOLVED);
	}

	unlink(tmpfname);

	if (write(fd, buf, BUF_SIZE) != BUF_SIZE) {
		printf(TNAME " Error at write(): %s\n", strerror(errno));
		exit(PTS_UNRESOLVED);
	}

	memset(&aiocb, 0, sizeof(struct aiocb));
	aiocb.aio_fildes = fd;
	aiocb.aio_buf = buf;
	aiocb.aio_offset = -1;
	aiocb.aio_nbytes = BUF_SIZE;

	if (aio_read(&aiocb) != -1) {
		struct timespec completion_wait_ts = {0, 10000000};
		int err;
		do {
			nanosleep(&completion_wait_ts, NULL);
			err = aio_error(&aiocb);
		} while (err == EINPROGRESS);

		int ret = aio_return(&aiocb);

		if (ret != -1) {
			printf(TNAME " bad aio_read return value\n");
			close(fd);
			exit(PTS_FAIL);
		} else if (err != EINVAL) {
			printf(TNAME " error code is not EINVAL %s\n",
			       strerror(errno));
			close(fd);
			exit(PTS_FAIL);
		}
	} else {

		if (errno != EINVAL) {
			printf(TNAME " errno is not EINVAL %s\n",
			       strerror(errno));
			close(fd);
			exit(PTS_FAIL);
		}
	}

	close(fd);
	printf("Test PASSED\n");
	return PTS_PASS;
}
