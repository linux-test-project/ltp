/*
 * Copyright (c) 2004, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 *  If pid=0, then clock_getcpuclockid() will return the CPU-time clock of
 *  the calling process.
 *
 */


#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "posixtest.h"
#include "timespec.h"

int main(void)
{
#if !defined(_POSIX_CPUTIME) || _POSIX_CPUTIME == -1
	printf("_POSIX_CPUTIME unsupported\n");
	return PTS_UNSUPPORTED;
#else
	clockid_t clockid_1, clockid_2;
	struct timespec t1, t2, t3;

	if (sysconf(_SC_CPUTIME) == -1) {
		printf("_POSIX_CPUTIME unsupported\n");
		return PTS_UNSUPPORTED;
	}

	if (clock_getcpuclockid(getpid(), &clockid_1) != 0) {
		printf("clock_getcpuclockid(getpid(),) failed\n");
		return PTS_FAIL;
	}

	if (clock_getcpuclockid(0, &clockid_2) != 0) {
		printf("clock_getcpuclockid(0,) failed\n");
		return PTS_FAIL;
	}

	/* if ids are the same, we are done */
	if (clockid_1 == clockid_2 && clockid_2 == CLOCK_PROCESS_CPUTIME_ID) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	/* otherwise get cputimes and check that they differs only a little */
	if (clock_gettime(clockid_1, &t1) != 0) {
		printf("clock_gettime(clockid_1,) failed\n");
		return PTS_FAIL;
	}

	if (clock_gettime(clockid_2, &t2) != 0) {
		printf("clock_gettime(clockid_2,) failed\n");
		return PTS_FAIL;
	}

	if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t3) != 0) {
		printf("clock_gettime(CLOCK_PROCESS_CPUTIME_ID,) failed\n");
		return PTS_FAIL;
	}

	if (timespec_nsec_diff(&t1, &t2) > NSEC_IN_SEC / 2 ||
	    timespec_nsec_diff(&t2, &t3) > NSEC_IN_SEC / 2) {
		printf("reported times differ too much\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
#endif
}
