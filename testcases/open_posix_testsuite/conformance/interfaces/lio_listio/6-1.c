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

#define TNAME "lio_listio/6-1.c"

int main()
{
#if _POSIX_ASYNCHRONOUS_IO != 200112L
	exit(PTS_UNSUPPORTED);
#endif

#ifndef LIO_READ
	printf(TNAME " LIO_READ not defined\n");
	exit(PTS_FAIL);
#endif

#ifndef LIO_WRITE
	printf(TNAME " LIO_WRITE not defined\n");
	exit(PTS_FAIL);
#endif

#ifndef LIO_WRITE
	printf(TNAME " LIO_WRITE not defined\n");
	exit(PTS_FAIL);
#endif

	printf ("Test PASSED\n");
	return PTS_PASS;
}
