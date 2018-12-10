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
 *	If the monotonic clock option is supported, the clock that shall be
 *	used to measure this time interval shall be CLOCK_MONOTONIC clock.
 *
 * method:
 *
 *	UNTESTED
 */

#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

int main(void)
{
	if (sysconf(_SC_ASYNCHRONOUS_IO) != 200112L ||
	    sysconf(_SC_MONOTONIC_CLOCK) < 200112L)
		exit(PTS_UNSUPPORTED);
	exit(PTS_UNTESTED);
}
