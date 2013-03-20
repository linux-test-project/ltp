/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test that clock_settime() sets errno = EINVAL if clock_id is
 * CLOCK_MONOTONIC.
 *
 * The date chosen is Nov 12, 2002 ~11:13am.
 */
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "posixtest.h"

#define TESTTIME 1037128358

int main(void)
{
#ifdef CLOCK_MONOTONIC
	struct timespec tpset;

	tpset.tv_sec = TESTTIME;
	tpset.tv_nsec = 0;
	if (geteuid() != 0) {
		printf("Test must be run as superuser\n");
		return PTS_UNRESOLVED;
	}
	if (clock_settime(CLOCK_MONOTONIC, &tpset) == -1) {
		if (EINVAL == errno) {
			printf("Test PASSED\n");
			return PTS_PASS;
		} else {
			printf("errno != EINVAL\n");
			return PTS_FAIL;
		}
	} else {
		printf("clock_settime() did not fail with CLOCK_MONOTONIC\n");
		return PTS_UNRESOLVED;
	}

	return PTS_UNRESOLVED;
#else
	printf("CLOCK_MONOTONIC not supported\n");
	return PTS_UNSUPPORTED;
#endif

}
