/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test that TIMER_MAX >= _POSIX_TIMER_MAX
 */

#include <limits.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "posixtest.h"

int main(void)
{
	long scTIMER_MAX = 0;

	scTIMER_MAX = sysconf(_SC_TIMER_MAX);

#ifdef DEBUG
	printf("TIMER_MAX = %ld\n_POSIX_TIMER_MAX=%ld\n",
	       scTIMER_MAX, (long)_POSIX_TIMER_MAX);
#endif

	if ((scTIMER_MAX != -1) && (scTIMER_MAX < _POSIX_TIMER_MAX)) {
		printf("Test FAILED (%ld < %ld)\n", scTIMER_MAX,
		       (long)_POSIX_TIMER_MAX);
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
