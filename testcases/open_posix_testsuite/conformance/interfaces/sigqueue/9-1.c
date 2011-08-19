/*
 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Updated: 21.06.2011  Peter W. Morreale <pmorreale@novell.com>
 *
 * Steps:
 * - Register for myhandler to be called when SIGTOTEST is called, and make
 *   sure SA_SIGINFO is set.
 * - Block signal SIGTOTEST from the process.
 * - Using sysconf(), check to see if there is a limit on number of queued
 *   signals that are pending. If there isn't a limit (i.e. sysconf returned
 *   -1), then this test is not applicable to the system's implementation,
 *   and thus we should pass it.
 * - Using sigqueue(), send to the current process a number of instances
 *   (of SIGTOTEST) equal to the limit that sysconf() returned.
 * - Send one more instance of SIGTOTEST and verify that sigqueue returns
 *   -1 and sets errno to [EAGAIN]
 *
 */

#define _XOPEN_SOURCE 600
#define SIGTOTEST SIGRTMIN

#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include "posixtest.h"

int main(void)
{
	int pid = getpid();
	int i;
	long syslimit;
	int rc;
	union sigval value;

	value.sival_int = 0;	/* 0 is just an arbitrary value */
	pid = getpid();

	sighold(SIGTOTEST);

	/*
	 * Get system limit.  Note that this limit is optional.
	 */
	syslimit = sysconf(_SC_SIGQUEUE_MAX);
	if (syslimit < 0)
		goto done;

	for (i = 0; i < syslimit; i++) {
		if (sigqueue(pid, SIGTOTEST, value) != 0) {
			printf("Failed: sigqueue on %d of %d max, errno: %s\n",
				i, syslimit, strerror(errno));
			return PTS_UNRESOLVED;
		}
	}

	/*
	 * Enqueue one more, needs to fail with EAGAIN
	 */
	rc = sigqueue(pid, SIGTOTEST, value);
	if (!(rc == -1 && errno == EAGAIN)) {
		printf("Failed: sigqueue() queued SIGQUEUE_MAX+1 signals\n");
		return PTS_FAIL;
	}

done:
	printf("Test PASSED\n");

	return PTS_PASS;
}
