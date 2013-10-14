/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that timer_gettime() sets itimerspec.it_value to the amount of
 * time remaining after it has expired once and is reloaded.  Also
 * test that timer_gettime() sets itimerspec.it_interval to the
 * interval remaining.
 * - Create and arm a timer for TIMERVALSEC seconds with interval
 *   TIMERINTERVALSEC seconds.
 * - Sleep for SLEEPSEC > TIMERVALSEC, but < TIMERINTERVAL seconds
 * - Call timer_gettime().
 * - Ensure the value returned is within ACCEPTABLEDELTA less than
 *   TIMERINTERVALSEC - (SLEEPSEC-TIMERVALSEC).
 * - Ensure that interval TIMERINTERVALSEC is returned.
 *
 * Signal SIGCONT will be used so that it will not affect the test if
 * the timer expires.
 * Clock CLOCK_REALTIME will be used.
 */

#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "posixtest.h"

#define TIMERVALSEC 2
#define TIMERINTERVALSEC 8
#define SLEEPSEC 3
#define ACCEPTABLEDELTA 1
#define RESOLUTION	1000000

int main(void)
{
	struct sigevent ev;
	timer_t tid;
	struct itimerspec itsset, itsget;
	int delta;
	int expectedleft;

	ev.sigev_notify = SIGEV_SIGNAL;
	ev.sigev_signo = SIGCONT;

	itsset.it_interval.tv_sec = TIMERINTERVALSEC;
	itsset.it_interval.tv_nsec = 2 * RESOLUTION;
	itsset.it_value.tv_sec = TIMERVALSEC;
	itsset.it_value.tv_nsec = 0;

	if (timer_create(CLOCK_REALTIME, &ev, &tid) != 0) {
		perror("timer_create() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if (timer_settime(tid, 0, &itsset, NULL) != 0) {
		perror("timer_settime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if (sleep(SLEEPSEC) != 0) {
		perror("sleep() did not return 0\n");
		return PTS_UNRESOLVED;
	}

	if (timer_gettime(tid, &itsget) != 0) {
		perror("timer_gettime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	/*
	 * Check interval first
	 */
	if ((itsget.it_interval.tv_sec != itsset.it_interval.tv_sec) ||
	    (itsget.it_interval.tv_nsec / RESOLUTION !=
	     itsset.it_interval.tv_nsec / RESOLUTION)) {
		printf("FAIL:  it_interval not correctly set\n");
		printf("%d != (expected) %d or %d != (expected) %d\n",
		       (int)itsget.it_interval.tv_sec,
		       (int)itsset.it_interval.tv_sec,
		       (int)itsget.it_interval.tv_nsec,
		       (int)itsset.it_interval.tv_nsec);
		return PTS_UNRESOLVED;
	}

	/*
	 * Check value next
	 * value should be < TIMERINTERVALSEC - (SLEEPSEC-TIMERVALSEC)
	 */
	expectedleft = itsset.it_interval.tv_sec -
	    (SLEEPSEC - itsset.it_value.tv_sec);
	delta = expectedleft - itsget.it_value.tv_sec;

	if (delta < 0) {
		printf("FAIL:  timer_gettime() value > time expected left\n");
		printf("%d > %d\n", (int)itsget.it_value.tv_sec, expectedleft);
		return PTS_FAIL;
	}

	if (delta <= ACCEPTABLEDELTA) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	printf("FAIL:  timer_gettime() value !~= time expected left\n");
	printf("%d !~= %d\n", (int)itsget.it_value.tv_sec,
	       (int)itsset.it_value.tv_sec - SLEEPSEC);
	return PTS_FAIL;
}
