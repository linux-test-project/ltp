/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test that timer_settime() resets the time until the next timer
 * expiration to value.it_value if the timer was already armed.
 *
 * - set up a timer with value.it_value = V and value.it_interval > 0
 * - ensure that timer expires correctly the first time
 * - change time value to value.it_value = V'
 * - ensure that this new timer expires correctly
 * - repeat above 2 steps NUMREPS times
 *
 * For this test, signal SIGTOTEST will be used, clock CLOCK_REALTIME
 * will be used.
 */

#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "posixtest.h"

#define SIGTOTEST SIGALRM
#define TIMERVALUESEC 2
#define TIMERINTERVALSEC 5
#define INCREMENT 2
#define ACCEPTABLEDELTA 1

#define NUMREPS 5

int main(void)
{
	struct sigevent ev;
	timer_t tid;
	struct itimerspec its;
	struct timespec tsbefore, tsafter;
	sigset_t set;
	int sig, i, failure = 0, timeelapsed, delta;

	/*
	 * set up signal set containing SIGTOTEST that will be used
	 * in call to sigwait immediately after timer is set
	 */

	if (sigemptyset(&set) == -1) {
		perror("sigemptyset() failed\n");
		return PTS_UNRESOLVED;
	}

	if (sigaddset(&set, SIGTOTEST) == -1) {
		perror("sigaddset() failed\n");
		return PTS_UNRESOLVED;
	}

	if (sigprocmask(SIG_BLOCK, &set, NULL) == -1) {
		perror("sigprocmask() failed\n");
		return PTS_UNRESOLVED;
	}

	/*
	 * set up timer to perform action SIGTOTEST on expiration
	 */
	ev.sigev_notify = SIGEV_SIGNAL;
	ev.sigev_signo = SIGTOTEST;

	its.it_interval.tv_sec = TIMERINTERVALSEC;
	its.it_interval.tv_nsec = 0;
	its.it_value.tv_sec = TIMERVALUESEC;
	its.it_value.tv_nsec = 0;

	if (timer_create(CLOCK_REALTIME, &ev, &tid) != 0) {
		perror("timer_create() did not return success\n");
		return PTS_UNRESOLVED;
	}

	/*
	 * - for i = 1 to NUMREPS
	 *    - get the time
	 *    - call timer_settime()
	 *    - wait for the signal
	 *    - get the time again => value.it_value
	 *    - set new value.it_value and value.it_interval to old + INVERVAL
	 * - delete the timer
	 */

	for (i = 0; i < NUMREPS; i++) {
		printf("Test for value %d\n", (int)its.it_value.tv_sec);
		if (clock_gettime(CLOCK_REALTIME, &tsbefore) != 0) {
			perror("clock_gettime() did not return success\n");
			return PTS_UNRESOLVED;
		}

		if (timer_settime(tid, 0, &its, NULL) != 0) {
			perror("timer_settime() did not return success\n");
			return PTS_UNRESOLVED;
		}

		if (sigwait(&set, &sig) == -1) {
			perror("sigwait() failed\n");
			return PTS_UNRESOLVED;
		}

		if (clock_gettime(CLOCK_REALTIME, &tsafter) != 0) {
			perror("clock_gettime() did not return success\n");
			return PTS_UNRESOLVED;
		}

		timeelapsed = tsafter.tv_sec - tsbefore.tv_sec;

		if (timeelapsed < 0) {
			perror("clock_gettime inconsistent\n");
			return PTS_UNRESOLVED;
		}

		delta = timeelapsed - its.it_value.tv_sec;
		if ((delta > ACCEPTABLEDELTA) || (delta < 0)) {
			printf("FAIL:  timer_settime() invalid on %d\n",
			       (int)its.it_value.tv_sec);
			failure = 1;
		}

		its.it_interval.tv_sec += INCREMENT;
		its.it_value.tv_sec += INCREMENT;
	}

	if (timer_delete(tid) != 0) {
		perror("timer_delete() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if (failure) {
		printf("timer_settime() failed on at least one value\n");
		return PTS_FAIL;
	} else {
		printf("Test PASSED\n");
		return PTS_PASS;
	}
}
