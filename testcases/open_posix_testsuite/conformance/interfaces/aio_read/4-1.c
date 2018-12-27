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
 * The file is read at offset given by aio_offset.
 *
 * method:
 *
 *	- write data to a file
 *	- read file using aio_read at a given offset
 *	- check data is consistent
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

#define TNAME "aio_read/4-1.c"

int main(void)
{
	char tmpfname[256];
#define BUF_SIZE 512
	unsigned char buf[BUF_SIZE * 2];
	unsigned char check[BUF_SIZE];
	int fd;
	struct aiocb aiocb;
	int i;
	struct timespec completion_wait_ts = {0, 10000000};

	if (sysconf(_SC_ASYNCHRONOUS_IO) < 200112L)
		return PTS_UNSUPPORTED;

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_aio_read_4_1_%d",
		 getpid());
	unlink(tmpfname);
	fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf(TNAME " Error at open(): %s\n", strerror(errno));
		exit(PTS_UNRESOLVED);
	}

	unlink(tmpfname);

	memset(&buf[0], 1, BUF_SIZE);
	memset(&buf[BUF_SIZE], 2, BUF_SIZE);

	if (write(fd, buf, BUF_SIZE * 2) != BUF_SIZE * 2) {
		printf(TNAME " Error at write(): %s\n", strerror(errno));
		exit(PTS_UNRESOLVED);
	}

	memset(check, 0xaa, BUF_SIZE);
	memset(&aiocb, 0, sizeof(struct aiocb));
	aiocb.aio_fildes = fd;
	aiocb.aio_buf = check;
	aiocb.aio_nbytes = BUF_SIZE;
	aiocb.aio_offset = BUF_SIZE;

	if (aio_read(&aiocb) == -1) {
		printf(TNAME " Error at aio_read(): %s\n", strerror(errno));
		exit(PTS_FAIL);
	}

	int err;
	int ret;

	/* Wait until end of transaction */
	do {
		nanosleep(&completion_wait_ts, NULL);
		err = aio_error(&aiocb);
	} while (err == EINPROGRESS);

	ret = aio_return(&aiocb);

	if (err != 0) {
		printf(TNAME " Error at aio_error() : %s\n", strerror(err));
		close(fd);
		exit(PTS_FAIL);
	}

	if (ret != BUF_SIZE) {
		printf(TNAME " Error at aio_return()\n");
		close(fd);
		exit(PTS_FAIL);
	}

	/* check it */
	for (i = 0; i < BUF_SIZE; i++) {
		if (check[i] != 2) {
			printf(TNAME " read values are corrupted\n");
			close(fd);
			exit(PTS_FAIL);
		}
	}

	close(fd);
	printf("Test PASSED\n");
	return PTS_PASS;
}
