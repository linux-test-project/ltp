/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_condattr_setclock()
 *
 *  If it is called with a clock_id argument that refers to a CPU-time clock, the call
 *  shall fail.
 *
 * Steps:
 * 1.  Initialize a pthread_condattr_t object
 * 2.  Get the cpu clock id
 * 3.  Call pthread_condattr_setclock passing this clock id to it
 * 4.  It should fail.
 *
 */


#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include "posixtest.h"

int main(void)
{

#if _POSIX_CPUTIME == -1
	printf("_POSIX_CPUTIME unsupported\n");
	return PTS_UNSUPPORTED;
#endif

	pthread_condattr_t condattr;
	clockid_t clockid;
	int rc;

	if (sysconf(_SC_CPUTIME) == -1) {
		printf("_POSIX_CPUTIME unsupported\n");
		return PTS_UNSUPPORTED;
	}

	/* Initialize a cond attributes object */
	if ((rc = pthread_condattr_init(&condattr)) != 0) {
		fprintf(stderr, "Error at pthread_condattr_init(), rc=%d\n",
			rc);
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	/* Get the cpu clock id */

	if (clock_getcpuclockid(getpid(), &clockid) != 0) {
		printf("clock_getcpuclockid() failed\n");
		return PTS_FAIL;
	}

	rc = pthread_condattr_setclock(&condattr, clockid);
	if (rc != EINVAL) {
		printf
		    ("Test FAILED: Expected EINVAL when passing a cpu clock id, instead it returned: %d \n",
		     rc);
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
