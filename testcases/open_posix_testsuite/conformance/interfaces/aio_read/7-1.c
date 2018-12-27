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
 * The aio_read() function shall return the value zero if operation is
 * successfuly queued.
 *
 * method:
 *
 *	- write data to a file
 *	- read file using aio_read
 *	- check aio_read return value
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

#define TNAME "aio_read/7-1.c"

int main(void)
{
	char tmpfname[256];
#define BUF_SIZE 111
	unsigned char check[BUF_SIZE];
	int fd;
	int ret;
	struct aiocb aiocb;
	struct timespec completion_wait_ts = {0, 10000000};

	if (sysconf(_SC_ASYNCHRONOUS_IO) < 200112L)
		return PTS_UNSUPPORTED;

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_aio_read_7_1_%d",
		 getpid());
	unlink(tmpfname);
	fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf(TNAME " Error at open(): %s\n", strerror(errno));
		exit(PTS_UNRESOLVED);
	}

	unlink(tmpfname);

	memset(check, 0xaa, BUF_SIZE);
	memset(&aiocb, 0, sizeof(struct aiocb));
	aiocb.aio_fildes = fd;
	aiocb.aio_buf = check;
	aiocb.aio_nbytes = BUF_SIZE;

	if (aio_read(&aiocb) == -1) {
		printf(TNAME " Error at aio_read(): %s\n", strerror(errno));
		exit(PTS_FAIL);
	}

	do {
		nanosleep(&completion_wait_ts, NULL);
		ret = aio_error(&aiocb);
	} while (ret == EINPROGRESS);

	close(fd);
	printf("Test PASSED\n");
	return PTS_PASS;
}
