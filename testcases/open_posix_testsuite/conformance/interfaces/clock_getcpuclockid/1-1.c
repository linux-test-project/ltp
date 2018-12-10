/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * @pt:CPT
 * General test that clock_getcpuclockid() returns CPU-time clock for a
 * process.  The process chosen is the current process.
 *
 *
 * 12/17/02 - Checking in correction made by
 *            jim.houston REMOVE-THIS AT attbi DOT com
 *            Test needed to do something as opposed to idle sleep to
 *            get the CPU time to increase.
 */
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "posixtest.h"

#define LARGENUMBER 900000
void dosomething(void)
{
	int i;
	for (i = 0; i < LARGENUMBER; i++) {
		clock();
	}
}

int main(void)
{
#if !defined(_POSIX_CPUTIME) || _POSIX_CPUTIME == -1
	printf("_POSIX_CPUTIME unsupported\n");
	return PTS_UNSUPPORTED;
#else
	struct timespec tp1;
	clockid_t clockid;

	if (sysconf(_SC_CPUTIME) == -1) {
		printf("_POSIX_CPUTIME unsupported\n");
		return PTS_UNSUPPORTED;
	}

	dosomething();

	if (clock_getcpuclockid(getpid(), &clockid) != 0) {
		printf("clock_getcpuclockid() failed\n");
		return PTS_FAIL;
	}

	/*
	 * Verify that it returned a valid clockid_t that can be used in other
	 * functions
	 */
	if (clock_gettime(clockid, &tp1) != 0) {
		printf
		    ("clock_getcpuclockid() returned an invalid clockid_t: %d\n",
		     clockid);
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
#endif
}
