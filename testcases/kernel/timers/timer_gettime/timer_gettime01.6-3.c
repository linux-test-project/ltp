/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 *
 * Test that timer_gettime() sets errno = EINVAL when timerid =
 * a timer ID of a deleted timer.
 *
 * For this test, signal SIGCONT will be used.
 * Clock CLOCK_REALTIME will be used.
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
	struct itimerspec its;

	ev.sigev_notify = SIGEV_SIGNAL;
	ev.sigev_signo = SIGCONT;

	if (timer_create(CLOCK_REALTIME, &ev, &tid) != 0) {
		perror("timer_create() did not return success\n");
		return PTS_UNRESOLVED;
	}
	if (timer_delete(tid) != 0) {
		perror("timer_delete() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if (timer_gettime(tid, &its) == -1) {
		if (EINVAL == errno) {
			printf("Test PASSED\n");
			return PTS_PASS;
		} else {
			printf("returned -1, but errno not set\n");
			return PTS_FAIL;
		}
	} else {
		printf("timer_settime() did not return failure\n");
		return PTS_UNRESOLVED;
	}

	printf("This code should not be executed\n");
	return PTS_UNRESOLVED;
}
