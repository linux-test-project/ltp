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
 * For regular files, no data transfer shall occur past the offset
 * maximum established in the open file description associated with
 * aio_fildes. 
 *
 * method:
 *
 *	- open file
 *	- write 1024 bytes using aio_write
 *	- Set RLIMIT_FSIZE to 512
 *	- try to read 1024 bytes using aio_read
 *	- check that only 512 bytes were read
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

#define TNAME "aio_read/6-1.c"

int main()
{
	char tmpfname[256];
#define BUF_SIZE 1024
	char buf[BUF_SIZE];
	char check[BUF_SIZE];
	int fd;
	struct aiocb aiocb;
	struct rlimit limit;
	int err;
	int ret;

#if _POSIX_ASYNCHRONOUS_IO != 200112L
	exit(PTS_UNSUPPORTED);
#endif

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_aio_write_4_1_%d", 
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

	memset(buf, 0xaa, BUF_SIZE);

	if (write(fd, buf, BUF_SIZE) != BUF_SIZE)
	{
		printf(TNAME " Error at write(): %s\n",
		       strerror(errno));
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

	memset(check, 0, BUF_SIZE);
	memset(&aiocb, 0, sizeof(struct aiocb));
	aiocb.aio_fildes = fd;
	aiocb.aio_buf = check;
	aiocb.aio_nbytes = BUF_SIZE;

	if (aio_read(&aiocb) == -1)
	{
		printf(TNAME " Error at aio_read(): %s\n",
		       strerror(errno));
		exit(PTS_FAIL);
	}

	/* Wait until end of transaction */
	while ((err = aio_error (&aiocb)) == EINPROGRESS);

	err = aio_error(&aiocb);
	ret = aio_return(&aiocb);

	if (err != 0)
	{
		printf(TNAME " Error at aio_error() : %s\n", strerror (err));
		close(fd);
		exit(PTS_FAIL);
	}

	if (ret != BUF_SIZE/2)
	{
		printf(TNAME " Error at aio_return()\n");
		close(fd);
		exit(PTS_FAIL);
	}

	/* check it */
	if (memcmp (buf, check, BUF_SIZE/2) != 0) {
		printf(TNAME " read values are corrupted\n");
		close(fd);
		exit(PTS_FAIL);
	}

	if (check[BUF_SIZE/2 + 1] != 0) {
		printf(TNAME " read past limit\n");
		close(fd);
		exit(PTS_FAIL);
	}

	close(fd);
	printf ("Test PASSED\n");
	return PTS_PASS;
}
