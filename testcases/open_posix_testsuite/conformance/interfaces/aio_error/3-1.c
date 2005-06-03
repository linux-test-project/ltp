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
 *	aio_error() shall fail if:
 *	[EINVAL] The aiocbp argument does not refer to an asynchronous
 *	operation whose return status has not yet been retrieved.
 *
 * method:
 *
 *	call aio_error() with an invalid aiocbp
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
#include <aio.h>

#include "posixtest.h"

#define TNAME "aio_error/3-1.c"

int main()
{
	struct aiocb bad;
	int ret;

#if _POSIX_ASYNCHRONOUS_IO != 200112L
	return PTS_UNSUPPORTED;
#endif

	memset (&bad, 0, sizeof (struct aiocb));

	ret = aio_error(&bad);
	if (ret != -1)
	{
		printf(TNAME " bad aio_error return value; %d\n", ret);
		return PTS_FAIL;
	}

	if (errno != EINVAL)
	{
		printf(TNAME " errno is not EINVAL %s\n", strerror(errno));
		return PTS_FAIL;
	}


	printf ("Test PASSED\n");
	return PTS_PASS;
}
