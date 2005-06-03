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
 *	For regular files, no data transfer shall occur past the offset
 *	maximum established in the open file description associated with
 *	aio_fildes.
 *
 * method:
 *
 *	- Set RLIMIT_FSIZE to 512
 *	- open file
 *	- write 1024 bytes using aio_write
 *	- check that only 512 bytes were written
 *	- try to read 1024 bytes
 *	- check that only 512 bytes were read
 *
 */

#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <aio.h>

#include "posixtest.h"

#define TNAME "aio_write/4-1.c"

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

	/* Set file size soft limit to 512 */
	getrlimit(RLIMIT_FSIZE , &limit);

	limit.rlim_cur = BUF_SIZE / 2;

	if (setrlimit(RLIMIT_FSIZE , &limit) == -1)
	{
		printf(TNAME " Error at setlimit(): %s\n",
		       strerror(errno));
		exit(PTS_UNRESOLVED);
	}

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
	memset(&aiocb, 0, sizeof(struct aiocb));
	aiocb.aio_fildes = fd;
	aiocb.aio_buf = buf;
	aiocb.aio_nbytes = BUF_SIZE;

	if (aio_write(&aiocb) == -1)
	{
		printf(TNAME " Error at aio_write(): %s\n",
		       strerror(errno));
		exit(PTS_FAIL);
	}

	/* Wait until completion */
	while (aio_error (&aiocb) == EINPROGRESS);

	err = aio_error(&aiocb);
	ret = aio_return(&aiocb);

	if (err != 0)
	{
		printf (TNAME " Error at aio_error() : %s\n", strerror (err));
		close (fd);
		exit(PTS_FAIL);
	}

	/* Check that written size did not exceed max value */
	if (ret != BUF_SIZE/2)
	{
		printf(TNAME " Error at aio_return()\n");
		close(fd);
		exit(PTS_FAIL);
	}

	/* check the values written */

	if (lseek(fd, 0, SEEK_SET) == -1)
	{
		printf(TNAME " Error at lseek(): %s\n",
		       strerror(errno));
		exit(PTS_FAIL);
	}

	/* we try to read more than we wrote to be sure of the size written */

	if (read(fd, check, BUF_SIZE) != BUF_SIZE / 2)
	{
		printf(TNAME " Error at read(): %s\n",
		       strerror(errno));
		exit(PTS_FAIL);
	}

	close(fd);
	printf ("Test PASSED\n");
	return PTS_PASS;
}
