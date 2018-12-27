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
 * aiocbp may be be used as an argument to aio_error() and aio_return().
 *
 * method:
 *
 *	- write data to a file
 *	- read file using aio_read
 *	- check error and return codes using previous aiocb
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

#define TNAME "aio_read/3-1.c"

int main(void)
{
	char tmpfname[256];
#define BUF_SIZE 111
	char buf[BUF_SIZE];
	int fd;
	int ret;
	struct aiocb aiocb;
	struct timespec completion_wait_ts = {0, 10000000};

	if (sysconf(_SC_ASYNCHRONOUS_IO) < 200112L)
		return PTS_UNSUPPORTED;

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_aio_read_3_1_%d",
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
	aiocb.aio_nbytes = BUF_SIZE;

	if (aio_read(&aiocb) == -1) {
		printf(TNAME " Error at aio_read(): %s\n", strerror(errno));
		exit(PTS_FAIL);
	}

	/* Wait for request completion */
	do {
		nanosleep(&completion_wait_ts, NULL);
		ret = aio_error(&aiocb);
	} while (ret == EINPROGRESS);

	/* error status shall be 0 and return status shall be BUF_SIZE */
	if (ret != 0) {
		printf(TNAME " Error at aio_error()\n");
		exit(PTS_FAIL);
	}

	if (aio_return(&aiocb) != BUF_SIZE) {
		printf(TNAME " Error at aio_return()\n");
		exit(PTS_FAIL);
	}

	close(fd);
	printf("Test PASSED\n");
	return PTS_PASS;
}
