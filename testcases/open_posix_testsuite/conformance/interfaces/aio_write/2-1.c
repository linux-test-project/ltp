/*
 * Copyright (c) 2004, IBM Corporation. All rights reserved.
 * Created by:  Laurent.Vivier@bull.net
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
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
#include <aio.h>

#include "posixtest.h"

#define TNAME "aio_write/2-1.c"

int main()
{
	char tmpfname[256];
#define BUF_SIZE0 400
	char buf0[BUF_SIZE0];
#define BUF_SIZE1 200
	char buf1[BUF_SIZE1];
#define BUF_SIZE2 300
	char buf2[BUF_SIZE2];
#define CHECK_SIZE (BUF_SIZE0+BUF_SIZE1+BUF_SIZE2)
	char check[CHECK_SIZE];
	int fd;
	struct aiocb aiocb0, aiocb1, aiocb2;

#if _POSIX_ASYNCHRONOUS_IO != 200112L
	exit(PTS_UNSUPPORTED);
#endif

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_aio_write_2_1_%d", 
		  getpid());
	unlink(tmpfname);
	fd = open(tmpfname, O_CREAT | O_APPEND | O_RDWR | O_EXCL,
		  S_IRUSR | S_IWUSR);
	if (fd == -1)
	{
		printf(TNAME " Error at open(): %s\n",
		       strerror(errno));
		exit(PTS_UNRESOLVED);
	}

	unlink(tmpfname);

	memset(buf0, 0x00, BUF_SIZE0);
	memset(&aiocb0, 0, sizeof(aiocb0));
	aiocb0.aio_fildes = fd;
	aiocb0.aio_buf = buf0;
	aiocb0.aio_nbytes = BUF_SIZE0;

	memset(buf1, 0x01, BUF_SIZE1);
	memset(&aiocb1, 0, sizeof(aiocb1));
	aiocb1.aio_fildes = fd;
	aiocb1.aio_buf = buf1;
	aiocb1.aio_nbytes = BUF_SIZE1;

	memset(buf2, 0x02, BUF_SIZE2);
	memset(&aiocb2, 0, sizeof(aiocb1));
	aiocb2.aio_fildes = fd;
	aiocb2.aio_buf = buf2;
	aiocb2.aio_nbytes = BUF_SIZE2;

	if (aio_write(&aiocb0) == -1)
	{
		printf(TNAME " Error at aio_write(): %s\n",
		       strerror(errno));
		exit(PTS_FAIL);
	}

	if (aio_write(&aiocb1) == -1)
	{
		printf(TNAME " Error at aio_write(): %s\n",
		       strerror(errno));
		exit(PTS_FAIL);
	}

	if (aio_write(&aiocb2) == -1)
	{
		printf(TNAME " Error at aio_write(): %s\n",
		       strerror(errno));
		exit(PTS_FAIL);
	}

	/* check the values written */

	if (lseek(fd, 0, SEEK_SET) == -1)
	{
		printf(TNAME " Error at lseek(): %s\n",
		       strerror(errno));
		exit(PTS_FAIL);
	}

	if (read(fd, check, CHECK_SIZE) != CHECK_SIZE)
	{
		printf(TNAME " Error at read(): %s\n",
		       strerror(errno));
		exit(PTS_FAIL);
	}

	if (memcmp(buf0, check, BUF_SIZE0))
	{
		printf(TNAME " Bad value in buffer #0\n");
		exit(PTS_FAIL);
	}

	if (memcmp(buf1, check + BUF_SIZE0, BUF_SIZE1))
	{
		printf(TNAME " Bad value in buffer #1\n");
		exit(PTS_FAIL);
	}

	if (memcmp(buf2, check + BUF_SIZE0 + BUF_SIZE1, BUF_SIZE2))
	{
		printf(TNAME " Bad value in buffer #2\n");
		exit(PTS_FAIL);
	}

	close(fd);
	printf ("Test PASSED\n");
	return PTS_PASS;
}
