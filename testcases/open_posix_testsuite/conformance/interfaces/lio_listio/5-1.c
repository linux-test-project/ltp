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

#define TNAME "lio_listio/5-1.c"

int main()
{
	struct aiocb aiocb;
	struct aiocb *list[1];

#if _POSIX_ASYNCHRONOUS_IO != 200112L
	exit(PTS_UNSUPPORTED);
#endif

	memset(&aiocb, 0, sizeof(aiocb));
	aiocb.aio_lio_opcode = -1;

	list[0] = &aiocb;

	if (lio_listio(LIO_WAIT, list, 1, NULL) != -1)
	{
		printf(TNAME " lio_listio() accepts invalid opcode\n");
		exit(PTS_FAIL);
	}

	printf ("Test PASSED\n");
	return PTS_PASS;
}
