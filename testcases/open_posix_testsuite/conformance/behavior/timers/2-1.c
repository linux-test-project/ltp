/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 *
 * Test that TIMER_MAX >= _POSIX_TIMER_MAX
 *
 * For this test, clock CLOCK_REALTIME will be used.
 */

#include <time.h>
#include <stdio.h>
#include <limits.h>
#include "posixtest.h"

int main(int argc, char *argv[])
{
#ifdef DEBUG
	printf("TIMER_MAX = %ld\n_POSIX_TIMER_MAX=%ld\n", 
			(long) TIMER_MAX, (long) _POSIX_TIMER_MAX);
#endif

	if (TIMER_MAX < _POSIX_TIMER_MAX) {
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
