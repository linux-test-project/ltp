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
 *	The aio_error() function shall return the error status (errno)
 *	associated with tha aiobcp argument.
 *
 * method:
 *
 *	execute aio_write()
 *	and check result with aio_error()
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

#include "posixtest.h"

#define TNAME "aio_error/1-1.c"

int main(void)
{
	char tmpfname[256];
#define BUF_SIZE 111
	char buf[BUF_SIZE];
	int fd, ret;
	struct aiocb aiocb;

	if (sysconf(_SC_ASYNCHRONOUS_IO) < 200112L)
		return PTS_UNSUPPORTED;

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_aio_error_1_1_%d",
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

	do {
		usleep(10000);
		ret = aio_error(&aiocb);
	} while (ret == EINPROGRESS);
	if (ret != 0) {
		printf(TNAME " Error at aio_error() : %s\n", strerror(ret));
		exit(PTS_FAIL);
	}

	close(fd);
	printf("Test PASSED\n");
	return PTS_PASS;
}
