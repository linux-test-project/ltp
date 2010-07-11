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
 *	lio_listio() shall fail if:
 *	[EAGAIN] The resource necessary to queue all the I/O request were not
 *	available.
 *
 * method:
 *
 *	UNTESTED
 *
 */

#define _XOPEN_SOURCE 600
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

int main(void)
{
	if (sysconf(_SC_ASYNCHRONOUS_IO) != 200112L)
		exit(PTS_UNSUPPORTED);
	exit(PTS_UNTESTED);
}
