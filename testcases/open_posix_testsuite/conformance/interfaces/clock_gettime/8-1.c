/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

   Test that clock_gettime() sets errno to EINVAL if clock_id does not
   specify a known clock.
 */
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include "posixtest.h"

#define INVALIDCLOCK 9999
int main(void)
{
	struct timespec tp;

	if (clock_gettime(INVALIDCLOCK, &tp) == -1) {
		if (EINVAL == errno) {
			printf("Test PASSED\n");
			return PTS_PASS;
		} else {
			printf("errno not set to EINVAL\n");
			printf("Test FAILED\n");
			return PTS_FAIL;
		}
	}

	printf("clock_gettime() did not return failure\n");
	return PTS_UNRESOLVED;
}
