/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

   General test that clock_gettime() returns a non-empty tp for a given
   clock_id (the clock_id chosen for this test is CLOCK_REALTIME).
 */
#include <stdio.h>
#include <time.h>
#include "posixtest.h"

int main(void)
{
	struct timespec tp;

	//Initialize tp
	tp.tv_sec = 0;
	tp.tv_nsec = 0;
	if (clock_gettime(CLOCK_REALTIME, &tp) == 0) {
		if (0 != tp.tv_sec) {	//assume this means time was sent
			printf("Test PASSED\n");
			return PTS_PASS;
		} else {
			printf("clock_gettime() success, but tp not filled\n");
			return PTS_FAIL;
		}
	}

	printf("clock_gettime() failed\n");
	return PTS_FAIL;
}
