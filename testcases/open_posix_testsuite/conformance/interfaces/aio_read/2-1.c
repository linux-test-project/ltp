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
 * If prioritized I/O is supported asynchronous operation shall be 
 * submitted at priority equal to a base schedulling priority minus 
 * aio_reqprio.
 *
 * method:
 *
 */

#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <aio.h>

#include "posixtest.h"

#define TNAME "aio_read/2-1.c"

int main()
{
#if _POSIX_ASYNCHRONOUS_IO != 200112L
	exit(PTS_UNSUPPORTED);
#endif

#ifndef _POSIX_PRIORITIZED_IO
	exit(PTS_UNSUPPORTED);
#endif

	return PTS_UNTESTED;
}
