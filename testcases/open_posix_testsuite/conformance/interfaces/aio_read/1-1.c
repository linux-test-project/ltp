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
 * aio_read() shall read aio_nbytes from the files aio_fildes into the
 * buffer aio_buf.
 *
 * method:
 *
 *	- write 1024 bytes into a file
 *	- read 256 bytes using aio_read
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

#define TNAME "aio_read/1-1.c"

int main(void)
{
	char tmpfname[256];
#define WBUF_SIZE 1024
	char buf[WBUF_SIZE];
#define RBUF_SIZE 256
	char check[RBUF_SIZE];
	int fd;
	int i;
	int err;
	int ret;

	struct aiocb aiocb;
	struct timespec completion_wait_ts = {0, 10000000};

	if (sysconf(_SC_ASYNCHRONOUS_IO) < 200112L)
		return PTS_UNSUPPORTED;

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_aio_read_1_1_%d",
		 getpid());
	unlink(tmpfname);
	fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf(TNAME " Error at open(): %s\n", strerror(errno));
		exit(PTS_UNRESOLVED);
	}

	unlink(tmpfname);

	for (i = 0; i < WBUF_SIZE / 256; i++)
		memset(&buf[i * 256], i + 1, 256);

	if (write(fd, buf, WBUF_SIZE) != WBUF_SIZE) {
		printf(TNAME " Error at write(): %s\n", strerror(errno));
		close(fd);
		exit(PTS_UNRESOLVED);
	}

	memset(&aiocb, 0, sizeof(struct aiocb));
	aiocb.aio_fildes = fd;
	aiocb.aio_buf = check;
	aiocb.aio_nbytes = RBUF_SIZE;
	aiocb.aio_offset = 512;

	if (aio_read(&aiocb) == -1) {
		printf(TNAME " Error at aio_read(): %s\n", strerror(errno));
		exit(PTS_FAIL);
	}

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

	if (ret != RBUF_SIZE) {
		printf(TNAME " Error at aio_return()\n");
		close(fd);
		exit(PTS_FAIL);
	}

	/* check it */
	if (memcmp(&buf[512], check, RBUF_SIZE) != 0) {
		printf(TNAME " read values are corrupted\n");
		close(fd);
		exit(PTS_FAIL);
	}

	close(fd);
	printf("Test PASSED\n");
	return PTS_PASS;
}
