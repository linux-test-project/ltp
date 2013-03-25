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
 * Test for a variety of timer values on absolute timers.
 * Steps:
 * - get time T0
 * - add offset to T0 to give T1
 * - set the timer to expire at T1
 * - wait for the timer to expire
 * - get time T2
 * - ensure T2 >= T1
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

static int timeroffsets[NUMTESTS][2] = { {0, 90000000}, {1, 0},
{1, 30000000}, {2, 0},
{3, 5000}, {4, 5}
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
	int flags = 0;

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

	flags |= TIMER_ABSTIME;
	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;
	for (i = 0; i < NUMTESTS; i++) {
		if (clock_gettime(CLOCK_REALTIME, &tsbefore) != 0) {
			perror("clock_gettime() did not return success\n");
			return PTS_UNRESOLVED;
		}

		if (tsbefore.tv_nsec + timeroffsets[i][1] < 1000000000) {
			its.it_value.tv_sec = tsbefore.tv_sec +
			    timeroffsets[i][0];
			its.it_value.tv_nsec = tsbefore.tv_nsec +
			    timeroffsets[i][1];
		} else {
			its.it_value.tv_sec = tsbefore.tv_sec +
			    timeroffsets[i][0] + 1;
			its.it_value.tv_nsec = tsbefore.tv_nsec +
			    timeroffsets[i][1] - 1000000000;
		}

		printf("Test for value %d sec %d nsec\n",
		       (int)its.it_value.tv_sec, (int)its.it_value.tv_nsec);

		if (timer_settime(tid, flags, &its, NULL) != 0) {
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

		printf("Timer expired %d sec %d nsec\n",
		       (int)tsafter.tv_sec, (int)tsafter.tv_nsec);
		if (tsafter.tv_sec < its.it_value.tv_sec) {
			printf("FAIL:  Timer expired %d sec < %d sec\n",
			       (int)tsafter.tv_sec, (int)its.it_value.tv_sec);
			failure = 1;
		} else if (tsafter.tv_sec == its.it_value.tv_sec) {
			if (tsafter.tv_nsec < its.it_value.tv_nsec) {
				printf
				    ("FAIL:  Timer expired %d nsec < %d nsec\n",
				     (int)tsafter.tv_nsec,
				     (int)its.it_value.tv_nsec);
				failure = 1;
			}
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
