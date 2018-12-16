/*
 * Copyright (c) 2010, Ngie Cooper.
 *
 * The clock_getcpuclockid() function may fail and return ESRCH if no process
 * can be found corresponding to the process specified by pid.
 *
 */


#include <sys/types.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "posixtest.h"

int main(void)
{
#if !defined(_POSIX_CPUTIME) || _POSIX_CPUTIME == -1
	printf("_POSIX_CPUTIME unsupported\n");
	return PTS_UNSUPPORTED;
#else
	clockid_t clockid_1;

	if (clock_getcpuclockid(-2, &clockid_1) == 0) {
		printf("clock_getcpuclockid(-2, ..) succeeded unexpectedly\n");
		return PTS_UNRESOLVED;
	}
	printf("Test PASSED\n");
	return PTS_PASS;

#endif
}
