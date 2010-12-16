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
 *	UNTESTED
 *
 */

#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <unistd.h>
#include <aio.h>

#include "posixtest.h"

int main()
{

#if _POSIX_ASYNCHRONOUS_IO != 200112L
	exit(PTS_UNSUPPORTED);
#endif

	return PTS_UNTESTED;
}