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

#define TNAME "aio_write/8-1.c"

int main()
{
#define BUF_SIZE 111
	char buf[BUF_SIZE];
	struct aiocb aiocb;

#if _POSIX_ASYNCHRONOUS_IO != 200112L
	exit(PTS_UNSUPPORTED);
#endif

	memset(buf, 0xaa, BUF_SIZE);
	memset(&aiocb, 0, sizeof(aiocb));
	aiocb.aio_fildes = -1;
	aiocb.aio_buf = buf;
	aiocb.aio_nbytes = BUF_SIZE;

	if (aio_write(&aiocb) != -1)
	{
		printf(TNAME " bad aio_write return value()\n");
		exit(PTS_FAIL);
	}

	if (errno != EBADF)
	{
		printf(TNAME " errno is not EBADF %s\n", strerror(errno));
		exit(PTS_FAIL);
	}


	printf ("Test PASSED\n");
	return PTS_PASS;
}
