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
 *	The error status of a succesfully queued  operation shall be:
	[ECANCELED] The requested I/O was canceled before the I/O completed.
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
