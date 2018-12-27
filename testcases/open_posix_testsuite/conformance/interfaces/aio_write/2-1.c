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
 *	If O_APPEND is set for the file descripton, write operations append to
 *	the file in the same order as the calls were made.
 *
 * method:
 *
 *	- open file in append mode
 *	- call aio_write thrice with different buffers and sizes
 *	- read all file
 *	- check read data
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

#define TNAME "aio_write/2-1.c"

int main(void)
{
	char tmpfname[256];
#define BUF_SIZE0 512
	char buf0[BUF_SIZE0];
#define BUF_SIZE1 1024
	char buf1[BUF_SIZE1];
#define BUF_SIZE2 1536
	char buf2[BUF_SIZE2];
	char *bufs[3] = { buf0, buf1, buf2 };
	ssize_t sizes[3] = { BUF_SIZE0, BUF_SIZE1, BUF_SIZE2 };
#define CHECK_SIZE (BUF_SIZE0+BUF_SIZE1+BUF_SIZE2)
	char check[CHECK_SIZE];
	int fd;
	struct aiocb aiocb[3];
	int i;
	int err;
	int ret;
	struct timespec completion_wait_ts = {0, 10000000};

	if (sysconf(_SC_ASYNCHRONOUS_IO) < 200112L)
		return PTS_UNSUPPORTED;

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_aio_write_2_1_%d",
		 getpid());
	unlink(tmpfname);
	fd = open(tmpfname, O_CREAT | O_APPEND | O_RDWR | O_EXCL,
		  S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf(TNAME " Error at open(): %s\n", strerror(errno));
		exit(PTS_UNRESOLVED);
	}

	unlink(tmpfname);

	for (i = 0; i < 3; i++) {
		memset(bufs[i], i, sizes[i]);
		memset(&aiocb[i], 0, sizeof(struct aiocb));
		aiocb[i].aio_fildes = fd;
		aiocb[i].aio_buf = bufs[i];
		aiocb[i].aio_nbytes = sizes[i];

		if (aio_write(&aiocb[i]) == -1) {
			printf(TNAME " Error at aio_write() %d: %s\n",
			       i, strerror(errno));
			close(fd);
			exit(PTS_FAIL);
		}
	}

	/* Wait until completion */
	do {
		nanosleep(&completion_wait_ts, NULL);
		err = aio_error(&aiocb[2]);
	} while (err == EINPROGRESS);

	for (i = 0; i < 3; i++) {
		err = aio_error(&aiocb[i]);
		ret = aio_return(&aiocb[i]);

		if (err != 0) {
			printf(TNAME " Error at aio_error() %d: %s\n", i,
			       strerror(err));
			close(fd);
			exit(PTS_FAIL);
		}

		if (ret != sizes[i]) {
			printf(TNAME " Error at aio_return() %d\n", i);
			close(fd);
			exit(PTS_FAIL);
		}
	}

	/* check the values written */

	if (lseek(fd, 0, SEEK_SET) == -1) {
		printf(TNAME " Error at lseek(): %s\n", strerror(errno));
		exit(PTS_FAIL);
	}

	if (read(fd, check, CHECK_SIZE) != CHECK_SIZE) {
		printf(TNAME " Error at read(): %s\n", strerror(errno));
		exit(PTS_FAIL);
	}

	if (memcmp(buf0, check, BUF_SIZE0)) {
		printf(TNAME " Bad value in buffer #0\n");
		exit(PTS_FAIL);
	}

	if (memcmp(buf1, check + BUF_SIZE0, BUF_SIZE1)) {
		printf(TNAME " Bad value in buffer #1\n");
		exit(PTS_FAIL);
	}

	if (memcmp(buf2, check + BUF_SIZE0 + BUF_SIZE1, BUF_SIZE2)) {
		printf(TNAME " Bad value in buffer #2\n");
		exit(PTS_FAIL);
	}

	close(fd);
	printf("Test PASSED\n");
	return PTS_PASS;
}
