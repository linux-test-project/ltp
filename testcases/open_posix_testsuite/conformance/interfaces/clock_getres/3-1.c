/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * pt:MON
 * Test that clock_getres() supports a clock_id of CLOCK_MONOTONIC if
 * pt:MON.
 */
#include <stdio.h>
#include <time.h>
#include "posixtest.h"

#define LARGENUM 100000

int main(void)
{
#ifdef CLOCK_MONOTONIC
	struct timespec res;

	/* Initialize res to a number much larger than the resolution
	 * could possibly be
	 */
	res.tv_sec = LARGENUM;
	res.tv_nsec = LARGENUM;
	if (clock_getres(CLOCK_MONOTONIC, &res) == 0) {
		if (res.tv_sec != LARGENUM) {	//assume initialized
#ifdef DEBUG
			printf("Resolution is %d sec %d nsec\n",
			       (int)res.tv_sec, (int)res.tv_nsec);
#endif
			printf("Test PASSED\n");
			return PTS_PASS;
		} else {
			printf("clock_getres() success, but res not filled\n");
		}
	} else {
		printf("clock_getres() failed\n");
	}

	printf("Test FAILED\n");
	return PTS_FAIL;
#else
	printf("CLOCK_MONOTONIC unsupported\n");
	return PTS_UNSUPPORTED;
#endif

}
