/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 *
 * Test that timer_getoverrun() sets errno = EINVAL for  timerid != a timer ID
 * created via timer_create().  [Try to set timerid to a timer ID
 * created + 1.]
 *
 * For this test, signal SIGCONT will be used, clock CLOCK_REALTIME
 * will be used.
 */

#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"


int main(int argc, char *argv[])
{
	struct sigevent ev;
	timer_t tid;

	ev.sigev_notify = SIGEV_SIGNAL;
	ev.sigev_signo = SIGCONT;

	if (timer_create(CLOCK_REALTIME, &ev, &tid) != 0) {
		perror("timer_create() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if (timer_getoverrun(tid+1) == -1) {
		if (EINVAL == errno) {
			printf("Test PASSED\n");
			return PTS_PASS;
		} else {
			printf("returned -1, but errno not set\n");
			return PTS_FAIL;
		}
	} else {
		printf("timer_getoverrun() did not return failure\n");
		return PTS_UNRESOLVED;
	}

	printf("This code should not be executed\n");
	return PTS_UNRESOLVED;
}
