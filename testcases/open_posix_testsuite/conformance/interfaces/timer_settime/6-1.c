/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test that if value.it_interval != 0, the timer is periodic and loaded
 * according to value.it_interval.
 *
 * - Set up a timer with an it_value < it_interval.
 * - Check that after it_value seconds, the timer expires.
 * - Check that after it_interval more seconds, the timer expires again (2X)
 * - Delete the timer
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
#define ACCEPTABLEDELTA 1

int main(void)
{
	struct sigevent ev;
	timer_t tid;
	struct itimerspec its;
	struct timespec tsbefore, tsafter;
	sigset_t set;
	int sig, i, delta, timeelapsed;

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
	 * - get the time
	 * - call timer_settime()
	 * - wait for the signal
	 * - get the time again => value.it_value
	 * - wait for signal again
	 * - get the time again => value.it_interval
	 * - repeat previous 2
	 * - delete the timer
	 */

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

	delta = timeelapsed - TIMERVALUESEC;

	if ((delta > ACCEPTABLEDELTA) || (delta < 0)) {
		perror("timer_settime() did not expire after value.it_value\n");
		return PTS_UNRESOLVED;
	}

	for (i = 0; i < 2; i++) {
		tsbefore.tv_sec = tsafter.tv_sec;
		tsbefore.tv_nsec = tsafter.tv_nsec;
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

		delta = timeelapsed - TIMERINTERVALSEC;

		if ((delta > ACCEPTABLEDELTA) || (delta < 0)) {
			printf("timer did not wait for correct interval\n");
			return PTS_FAIL;
		}
	}

	if (timer_delete(tid) != 0) {
		perror("timer_delete() did not return success\n");
		return PTS_UNRESOLVED;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
