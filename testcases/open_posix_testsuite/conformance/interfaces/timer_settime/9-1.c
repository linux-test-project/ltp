/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test that timers are not allowed to expire before their scheduled
 * time.
 *
 * Test for a variety of timer values on relative timers.
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
#define INCREMENT 1
#define ACCEPTABLEDELTA 1

#define NUMTESTS 6

static int timeroffsets[NUMTESTS][2] = { {0, 30000000}, {1, 0},
{1, 30000000}, {2, 0},
{1, 5000}, {1, 5}
};

int main(void)
{
	struct sigevent ev;
	timer_t tid;
	struct itimerspec its;
	struct timespec tsbefore, tsafter;
	sigset_t set;
	int sig;
	int i;
	int failure = 0;
	unsigned long totalnsecs, testnsecs;	// so long was we are < 2.1 seconds, we should be safe

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

	if (timer_create(CLOCK_REALTIME, &ev, &tid) != 0) {
		perror("timer_create() did not return success\n");
		return PTS_UNRESOLVED;
	}

	for (i = 0; i < NUMTESTS; i++) {
		its.it_interval.tv_sec = 0;
		its.it_interval.tv_nsec = 0;
		its.it_value.tv_sec = timeroffsets[i][0];
		its.it_value.tv_nsec = timeroffsets[i][1];

		printf("Test for value %d sec %d nsec\n",
		       (int)its.it_value.tv_sec, (int)its.it_value.tv_nsec);

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

		totalnsecs = (unsigned long)(tsafter.tv_sec - tsbefore.tv_sec) *
		    1000000000 + (tsafter.tv_nsec - tsbefore.tv_nsec);
		testnsecs = (unsigned long)its.it_value.tv_sec * 1000000000 +
		    its.it_value.tv_nsec;
		printf("total %lu test %lu\n", totalnsecs, testnsecs);
		if (totalnsecs < testnsecs) {
			printf("FAIL:  Expired %ld < %ld\n", totalnsecs,
			       testnsecs);
			failure = 1;
		}
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
