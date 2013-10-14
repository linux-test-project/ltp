/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

   Test that parameter CLOCK_MONOTONIC returns seconds since the
   an unspecified point that cannot change.
   Validity is checked by ensuring that the time returned is always
   increasing.
 */
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include "posixtest.h"

int main(void)
{
#ifdef CLOCK_MONOTONIC

	struct timespec ts1, ts2, ts3, ts4;

	/* Test that MONOTONIC CLOCK functionality really exists */
	if (sysconf(_SC_MONOTONIC_CLOCK) == -1) {
		printf("CLOCK_MONOTONIC unsupported\n");
		return PTS_UNSUPPORTED;
	}

	if (clock_gettime(CLOCK_MONOTONIC, &ts1) != 0) {
		printf("clock_gettime() failed\n");
		return PTS_UNRESOLVED;
	}

	if (clock_gettime(CLOCK_MONOTONIC, &ts2) != 0) {
		printf("clock_gettime() failed\n");
		return PTS_UNRESOLVED;
	}

	sleep(1);

	if (clock_gettime(CLOCK_MONOTONIC, &ts3) != 0) {
		printf("clock_gettime() failed\n");
		return PTS_UNRESOLVED;
	}

	sleep(3);
	if (clock_gettime(CLOCK_MONOTONIC, &ts4) != 0) {
		printf("clock_gettime() failed\n");
		return PTS_UNRESOLVED;
	}

	if ((ts1.tv_sec <= ts2.tv_sec) &&
	    (ts2.tv_sec <= ts3.tv_sec) && (ts3.tv_sec <= ts4.tv_sec)) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	printf("Test FAILED - ts1=%ld,ts2=%ld,ts3=%ld,ts4=%ld\n",
	       ts1.tv_sec, ts2.tv_sec, ts3.tv_sec, ts4.tv_sec);
	return PTS_FAIL;
#else
	printf("CLOCK_MONOTONIC unsupported\n");
	return PTS_UNSUPPORTED;
#endif
}
