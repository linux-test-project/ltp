/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

   Test that parameter CLOCK_PROCESS_CPUTIME_ID returns the CPU time of
   the calling process.
   Validity is checked by ensuring that the time returned is always
   increasing.
   This is only supported if _POSIX_CPUTIME is defined.
 */
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>
#include "posixtest.h"

#define LARGENUMBER 900000
void dosomething()
{
	int i;
	for (i = 0; i < LARGENUMBER; i++) {
		clock();
	}
}

int main(void)
{
#if _POSIX_CPUTIME == -1
	printf("_POSIX_CPUTIME unsupported\n");
	return PTS_UNSUPPORTED;
#else
#ifdef CLOCK_PROCESS_CPUTIME_ID
	struct timespec ts1, ts2, ts3, ts4;

	if (sysconf(_SC_CPUTIME) == -1) {
		printf("_POSIX_CPUTIME unsupported\n");
		return PTS_UNSUPPORTED;
	}

	if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts1) != 0) {
		printf("clock_gettime() failed: errno %d\n", errno);
		return PTS_UNRESOLVED;
	}

	dosomething();

	if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts2) != 0) {
		printf("clock_gettime() failed: errno %d\n", errno);
		return PTS_UNRESOLVED;
	}

	dosomething();

	if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts3) != 0) {
		printf("clock_gettime() failed: errno %d\n", errno);
		return PTS_UNRESOLVED;
	}

	dosomething();

	if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts4) != 0) {
		printf("clock_gettime() failed: errno %d\n", errno);
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
	printf("CLOCK_PROCESS_CPUTIME_ID unsupported\n");
	return PTS_UNSUPPORTED;
#endif
#endif
}
