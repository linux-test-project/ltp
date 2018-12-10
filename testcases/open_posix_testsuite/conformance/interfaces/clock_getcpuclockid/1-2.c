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
 * If the process described by pid exists and the calling process has
 * permission, the clock ID of this clock shall be returned in clock_id.
 *
 */
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "posixtest.h"

int main(void)
{
#if !defined(_POSIX_CPUTIME) || _POSIX_CPUTIME == -1
	printf("_POSIX_CPUTIME unsupported\n");
	return PTS_UNSUPPORTED;
#else
	struct timespec tp1;
	clockid_t clockid;
	int error;

	if (sysconf(_SC_CPUTIME) == -1) {
		printf("_POSIX_CPUTIME unsupported\n");
		return PTS_UNSUPPORTED;
	}

	error = clock_getcpuclockid(1, &clockid);
	if (error != 0) {
		printf("clock_getcpuclockid() failed: %s\n", strerror(error));
		return PTS_UNRESOLVED;
	}

	/*
	 * Verify that it returned a valid clockid_t that can be used in other
	 * functions
	 */
	if (clock_gettime(clockid, &tp1) != 0) {
		printf("clock_getcpuclockid() returned an invalid clockid_t: "
		       "%d\n", clockid);
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
#endif
}
