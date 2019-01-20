/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that timer_settime() sets the time until the next timer
 * expiration to value.it_value and arms the timer for a variety
 * of value.it_value values.
 *
 * Same steps as in 1-1.c; however, this time for multiple
 * value.it_value values.
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
#define SLEEPDELTA 3
#define ACCEPTABLEDELTA 1
#define NUMTESTS 6

void handler(int signo LTP_ATTRIBUTE_UNUSED)
{
	printf("Caught signal\n");
}

int main(void)
{
	struct sigevent ev;
	struct sigaction act;
	timer_t tid;
	struct itimerspec its;
	struct timespec ts, tsleft;
	int timervals[NUMTESTS][2] = {
		{0, 30000000}, {1, 0},
		{1, 30000000}, {2, 0},
		{10, 5000}, {13, 5}
	};
	int i;
	int failure = 0;

	ev.sigev_notify = SIGEV_SIGNAL;
	ev.sigev_signo = SIGTOTEST;

	act.sa_handler = handler;
	act.sa_flags = 0;

	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;

	if (sigemptyset(&act.sa_mask) == -1) {
		perror("Error calling sigemptyset\n");
		return PTS_UNRESOLVED;
	}
	if (sigaction(SIGTOTEST, &act, 0) == -1) {
		perror("Error calling sigaction\n");
		return PTS_UNRESOLVED;
	}

	for (i = 0; i < NUMTESTS; i++) {
		its.it_value.tv_sec = timervals[i][0];
		its.it_value.tv_nsec = timervals[i][1];

		ts.tv_sec = timervals[i][0] + SLEEPDELTA;
		ts.tv_nsec = timervals[i][1];

		if (timer_create(CLOCK_REALTIME, &ev, &tid) != 0) {
			perror("timer_create() did not return success\n");
			return PTS_UNRESOLVED;
		}

		if (timer_settime(tid, 0, &its, NULL) != 0) {
			perror("timer_settime() did not return success\n");
			failure = 1;
		}

		if (nanosleep(&ts, &tsleft) != -1) {
			perror("nanosleep() not interrupted\n");
			failure = 1;
		}

		if (labs(tsleft.tv_sec - SLEEPDELTA) > ACCEPTABLEDELTA) {
			printf("Timer did not last correct amount of time\n");
			failure = 1;
		}

		if (timer_delete(tid) == -1) {
			perror("timer_delete() returned failure\n");
			/*
			 * do not continue
			 */
			return PTS_UNRESOLVED;
		}
	}

	if (failure) {
		printf("At least one test FAILED\n");
		return PTS_FAIL;
	}

	printf("All tests PASSED\n");
	return PTS_PASS;
}
