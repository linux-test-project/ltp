/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test that timer_gettime() sets itimerspec.it_value = 0 if
 * the timer was previously disarmed because it had just expired with
 * no repeating interval.
 *
 * For this test, signal SIGCONT will be used so that the test will
 * not abort.  Clock CLOCK_REALTIME will be used.
 */

#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "posixtest.h"

#define TIMERSEC 1
#define SLEEPDELTA 1

int main(void)
{
	struct sigevent ev;
	timer_t tid;
	struct itimerspec itsset, itsget;

	ev.sigev_notify = SIGEV_SIGNAL;
	ev.sigev_signo = SIGCONT;

	itsset.it_interval.tv_sec = 0;
	itsset.it_interval.tv_nsec = 0;
	itsset.it_value.tv_sec = TIMERSEC;
	itsset.it_value.tv_nsec = 0;

	if (timer_create(CLOCK_REALTIME, &ev, &tid) != 0) {
		perror("timer_create() did not return success\n");
		return PTS_UNRESOLVED;
	}

	/*
	 * set up timer that will expire
	 */
	if (timer_settime(tid, 0, &itsset, NULL) != 0) {
		perror("timer_settime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	/*
	 * let timer expire (just call sleep())
	 */
	sleep(TIMERSEC + SLEEPDELTA);

	if (timer_gettime(tid, &itsget) != 0) {
		perror("timer_gettime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if (0 == itsget.it_value.tv_sec && 0 == itsget.it_value.tv_nsec) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	printf("Test FAILED:  tv_sec %d tv_nsec %d\n",
	       (int)itsget.it_value.tv_sec,
	       (int)itsget.it_value.tv_nsec);
	return PTS_FAIL;
}
