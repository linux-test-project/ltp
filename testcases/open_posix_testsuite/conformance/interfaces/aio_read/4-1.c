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

#define TNAME "aio_read/4-1.c"

int main()
{
	char tmpfname[256];
#define BUF_SIZE 128
	unsigned char buf[BUF_SIZE*2];
	unsigned char check[BUF_SIZE];
	int fd;
	struct aiocb aiocb;
	int i;

#if _POSIX_ASYNCHRONOUS_IO != 200112L
	exit(PTS_UNSUPPORTED);
#endif

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_aio_read_4_1_%d", 
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

	for (i = 0; i < BUF_SIZE*2; i++)
		buf[i] = i;

	if (write(fd, buf, BUF_SIZE*2) != BUF_SIZE*2)
	{
		printf(TNAME " Error at write(): %s\n",
		       strerror(errno));
		exit(PTS_UNRESOLVED);
	}

	unlink(tmpfname);

	memset(check, 0xaa, BUF_SIZE);
	memset(&aiocb, 0, sizeof(aiocb));
	aiocb.aio_fildes = fd;
	aiocb.aio_buf = check;
	aiocb.aio_nbytes = BUF_SIZE;
	aiocb.aio_offset = BUF_SIZE/2;

	if (aio_read(&aiocb) == -1)
	{
		printf(TNAME " Error at aio_read(): %s\n",
		       strerror(errno));
		exit(PTS_FAIL);
	}

	/* how to synchronize without using sig or aio_suspend ? */

	while (check[0] == 0xaa);

	/* check it */

	for (i = 0; i < BUF_SIZE; i++)
	{
		if (buf[i + BUF_SIZE/2] != check[i])
		{
			printf(TNAME " read values are corrupted\n");
			exit(PTS_FAIL);
		}
	}

	close(fd);
	printf ("Test PASSED\n");
	return PTS_PASS;
}
