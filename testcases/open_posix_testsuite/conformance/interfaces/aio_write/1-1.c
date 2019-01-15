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
 *	The aio_write() function shall write aio_nbytes to the files associated
 *	with aio_fildes from the buffer pointer to by aio_buf.
 *
 * method:
 *
 *	- open file
 *	- write 512 bytes using aio_write
 *	- read 513 bytes
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

#define TNAME "aio_write/1-1.c"

int main(void)
{
	char tmpfname[256];
#define BUF_SIZE 512
	char buf[BUF_SIZE];
	char check[BUF_SIZE + 1];
	int fd;
	struct aiocb aiocb;
	int err;
	int ret;
	struct timespec completion_wait_ts = {0, 10000000};

	if (sysconf(_SC_ASYNCHRONOUS_IO) < 200112L)
		return PTS_UNSUPPORTED;

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_aio_write_1_1_%d",
		 getpid());
	unlink(tmpfname);
	fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf(TNAME " Error at open(): %s\n", strerror(errno));
		exit(PTS_UNRESOLVED);
	}

	unlink(tmpfname);

	memset(buf, 0xaa, BUF_SIZE);
	memset(&aiocb, 0, sizeof(struct aiocb));
	aiocb.aio_fildes = fd;
	aiocb.aio_buf = buf;
	aiocb.aio_nbytes = BUF_SIZE;

	if (aio_write(&aiocb) == -1) {
		printf(TNAME " Error at aio_write(): %s\n", strerror(errno));
		close(fd);
		exit(PTS_FAIL);
	}

	/* Wait until completion */
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

	/* check the values written */

	if (lseek(fd, 0, SEEK_SET) == -1) {
		printf(TNAME " Error at lseek(): %s\n", strerror(errno));
		close(fd);
		exit(PTS_FAIL);
	}

	/* we try to read more than we wrote to be sure of the size written */

	check[BUF_SIZE] = 1;
	if (read(fd, check, BUF_SIZE + 1) != BUF_SIZE) {
		printf(TNAME " Error at read(): %s\n", strerror(errno));
		close(fd);
		exit(PTS_FAIL);
	}

	if (check[BUF_SIZE] != 1) {
		printf(TNAME " Buffer overflow\n");
		close(fd);
		exit(PTS_FAIL);
	}

	if (memcmp(buf, check, BUF_SIZE)) {
		printf(TNAME " Bad value in buffer\n");
		close(fd);
		exit(PTS_FAIL);
	}

	close(fd);
	printf("Test PASSED\n");
	return PTS_PASS;
}
