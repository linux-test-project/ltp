/*
 * Copyright (c) 2004, Bull SA. All rights reserved.
 * Created by:  Laurent.Vivier@bull.net
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
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

#define TNAME "aio_fsync/8-4.c"

int main(void)
{
	char tmpfname[256];
#define BUF_SIZE 111
	char buf[BUF_SIZE];
	int fd;
	int ret;
	struct aiocb aiocb_write;
	struct aiocb aiocb_fsync;
	struct timespec aio_wait_ts = {0, 10000000};

	if (sysconf(_SC_ASYNCHRONOUS_IO) < 200112L)
		return PTS_UNSUPPORTED;

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_aio_fsync_8_4_%d",
		 getpid());
	unlink(tmpfname);
	fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf(TNAME " Error at open(): %s\n", strerror(errno));
		exit(PTS_UNRESOLVED);
	}

	unlink(tmpfname);

	memset(&aiocb_write, 0, sizeof(aiocb_write));
	aiocb_write.aio_fildes = fd;
	aiocb_write.aio_buf = buf;
	aiocb_write.aio_nbytes = BUF_SIZE;

	if (aio_write(&aiocb_write) == -1) {
		printf(TNAME " Error at aio_write(): %s\n", strerror(errno));
		exit(PTS_FAIL);
	}

	do {
		nanosleep(&aio_wait_ts, NULL);
		ret = aio_error(&aiocb_write);
	} while (ret == EINPROGRESS);
	if (ret < 0) {
		printf(TNAME " Error at aio_error() : %s\n", strerror(ret));
		exit(PTS_FAIL);
	}

	memset(&aiocb_fsync, 0, sizeof(aiocb_fsync));
	aiocb_fsync.aio_fildes = fd;
	aiocb_fsync.aio_offset = -1;

	if (aio_fsync(O_SYNC, &aiocb_fsync) != 0) {
		printf(TNAME " Error at aio_fsync(): %s\n", strerror(errno));
		exit(PTS_FAIL);
	}

	/* wait for aio_fsync */
	do {
		nanosleep(&aio_wait_ts, NULL);
		ret = aio_error(&aiocb_fsync);
	} while (ret == EINPROGRESS);

	ret = aio_return(&aiocb_fsync);
	if (ret) {
		printf(TNAME " Error at aio_return(): %d (%s)\n",
			ret, strerror(errno));
		close(fd);
		return PTS_FAIL;
	}

	close(fd);
	printf("Test PASSED\n");
	return PTS_PASS;
}
