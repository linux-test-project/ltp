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

#define _XOPEN_SOURCE 600

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "posixtest.h"

int main(int argc, char *argv[])
{
#if !defined(_POSIX_CPUTIME) || _POSIX_CPUTIME == -1
        printf("_POSIX_CPUTIME unsupported\n");
        return PTS_UNSUPPORTED;
#else
	unsigned long time_to_set;	
	clockid_t clockid_1, clockid_2;

	if (sysconf(_SC_CPUTIME) == -1) {
		printf("_POSIX_CPUTIME unsupported\n");
		return PTS_UNSUPPORTED;
	}

	if (clock_getcpuclockid(getpid(), &clockid_1) != 0) {
		printf("clock_getcpuclockid(getpid(), ) failed\n");
		return PTS_FAIL;
	}

	if (clock_getcpuclockid(0, &clockid_2) != 0) {
		printf("clock_getcpuclockid(0, ) failed\n");
		return PTS_FAIL;
	}

	/* Get the time of clockid_1. */
	if (clockid_1 != clockid_2) {
		printf("clock_getcpuclockid(0, ..) != "
		    "clock_getcpuclockid(%d, ..): (%d != %d)\n", clockid_1,
		    clockid_2);
		return PTS_FAIL;
	}
	printf("Test PASSED\n");
	return PTS_PASS;

#endif
}
