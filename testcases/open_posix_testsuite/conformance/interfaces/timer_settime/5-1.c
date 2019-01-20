/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that timer_settime() uses an absolute clock if TIMER_ABSTIME is
 * set in flags.
 *
 * Uses the same steps as 1-1.c to determine how long the timer lasted.
 * In order to use an absolute clock, the timer time is determined by:
 * - Get current time.
 * - Set the timer to TIMERSEC+current time.
 * - After the timer goes off, ensure that the timer went off within
 *   ACCEPTABLEDELTA of TIMERSEC seconds.
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
#define TIMERSEC 2
#define SLEEPDELTA 3
#define ACCEPTABLEDELTA 1

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
	struct timespec beforets, ts, tsleft;
	int flags = 0;

	ev.sigev_notify = SIGEV_SIGNAL;
	ev.sigev_signo = SIGTOTEST;

	act.sa_handler = handler;
	act.sa_flags = 0;

	if (sigemptyset(&act.sa_mask) == -1) {
		perror("Error calling sigemptyset\n");
		return PTS_UNRESOLVED;
	}
	if (sigaction(SIGTOTEST, &act, 0) == -1) {
		perror("Error calling sigaction\n");
		return PTS_UNRESOLVED;
	}

	if (timer_create(CLOCK_REALTIME, &ev, &tid) != 0) {
		perror("timer_create() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if (clock_gettime(CLOCK_REALTIME, &beforets) != 0) {
		perror("clock_gettime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;
	its.it_value.tv_sec = beforets.tv_sec + TIMERSEC;
	its.it_value.tv_nsec = beforets.tv_nsec;

	ts.tv_sec = TIMERSEC + SLEEPDELTA;
	ts.tv_nsec = 0;

	flags |= TIMER_ABSTIME;
	if (timer_settime(tid, flags, &its, NULL) != 0) {
		perror("timer_settime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if (nanosleep(&ts, &tsleft) != -1) {
		perror("nanosleep() not interrupted\n");
		return PTS_FAIL;
	}

	if (labs(tsleft.tv_sec - SLEEPDELTA) <= ACCEPTABLEDELTA) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	printf("Timer did not last for correct amount of time\n");
	printf("timer: %d != correct %d\n",
	       (int)ts.tv_sec - (int)tsleft.tv_sec, TIMERSEC);
	return PTS_FAIL;
}
