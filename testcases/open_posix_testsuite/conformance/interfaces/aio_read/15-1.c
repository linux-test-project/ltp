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
 * aio_read() shall fail with:
 * [EOVERFLOW] if aio_nbytes is greater than 0, aio_offset is before EOF
 * and is at or beyond the offset maximum associated with aio_fildes.
 *
 * method:
 *
 *	- write BUF_SIZE bytes to a file
 *	- drop RLIMIT_FSIZE to BUF_SIZE/2
 *	- read BUF_SIZE/2 bytes at offset BUF_SIZE/2
 *	- check error code
 *
 */

#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <aio.h>

#include "posixtest.h"

#define TNAME "aio_read/15-1.c"

int main()
{
	char tmpfname[256];
#define BUF_SIZE 1024
	char buf[BUF_SIZE];
	int fd;
	struct aiocb aiocb;
	struct rlimit limit;

#if _POSIX_ASYNCHRONOUS_IO != 200112L
	exit(PTS_UNSUPPORTED);
#endif

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_aio_write_13_1_%d", 
		  getpid());
	unlink(tmpfname);
	fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL,
		  S_IRUSR | S_IWUSR);
	if (fd == -1)
	{
		printf(TNAME " Error at open(): %s\n",
		       strerror(errno));
		exit(PTS_UNRESOLVED);
	}

	unlink(tmpfname);

	if (write(fd, buf, BUF_SIZE) != BUF_SIZE)
	{
		printf(TNAME " Error at write(): %s\n",
		       strerror(errno));
		close(fd);
		exit(PTS_UNRESOLVED);
	}

	/* Set file size soft limit to 512 */
	getrlimit(RLIMIT_FSIZE , &limit);

	limit.rlim_cur = BUF_SIZE / 2;

	if (setrlimit(RLIMIT_FSIZE , &limit) == -1)
	{
		printf(TNAME " Error at setlimit(): %s\n",
		       strerror(errno));
		exit(PTS_UNRESOLVED);
	}

	/* try to read BUF_SIZE/2 bytes at offset BUF_SIZE/2 */
	memset(&aiocb, 0, sizeof(struct aiocb));
	aiocb.aio_fildes = fd;
	aiocb.aio_buf = buf;
	aiocb.aio_nbytes = BUF_SIZE/2;
	aiocb.aio_offset = BUF_SIZE/2;

	if (aio_read(&aiocb) != -1)
	{
		while (aio_error (&aiocb) == EINPROGRESS);

		int err = aio_error (&aiocb);
		int ret = aio_return (&aiocb);

		if (ret != -1) {
			printf(TNAME " bad aio_read return value\n");
			close (fd);
			exit(PTS_FAIL);
		} else if (err != EOVERFLOW) {
			printf(TNAME " error code is not EOVERFLOW %s\n", strerror(errno));
			close (fd);
			exit(PTS_FAIL);
		}
	}  else {

		if (errno != EOVERFLOW)
		{
			printf(TNAME " errno is not EOVERFLOW %s\n", strerror(errno));
			close(fd);
			exit(PTS_FAIL);
		}
	}

	close(fd);
	printf ("Test PASSED\n");
	return PTS_PASS;
}
