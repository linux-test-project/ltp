/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * pt:MON
 *
 * Test that CLOCK_MONOTONIC is supported by timer_create().
 *
 * Same test as 1-1.c with CLOCK_MONOTONIC.
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
#ifdef CLOCK_MONOTONIC
	if (sysconf(_SC_MONOTONIC_CLOCK) == -1) {
		printf("CLOCK_MONOTONIC unsupported\n");
		return PTS_UNSUPPORTED;
	}
	struct sigevent ev;
	struct sigaction act;
	timer_t tid;
	struct itimerspec its;
	struct timespec ts, tsleft;

	ev.sigev_notify = SIGEV_SIGNAL;
	ev.sigev_signo = SIGTOTEST;

	act.sa_handler = handler;
	act.sa_flags = 0;

	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;
	its.it_value.tv_sec = TIMERSEC;
	its.it_value.tv_nsec = 0;

	ts.tv_sec = TIMERSEC + SLEEPDELTA;
	ts.tv_nsec = 0;

	if (sigemptyset(&act.sa_mask) == -1) {
		perror("Error calling sigemptyset\n");
		return PTS_UNRESOLVED;
	}
	if (sigaction(SIGTOTEST, &act, 0) == -1) {
		perror("Error calling sigaction\n");
		return PTS_UNRESOLVED;
	}

	if (timer_create(CLOCK_MONOTONIC, &ev, &tid) != 0) {
		perror("timer_create() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if (timer_settime(tid, 0, &its, NULL) != 0) {
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
	} else {
		printf("Timer did not last for correct amount of time\n");
		printf("timer: %d != correct %d\n",
		       (int)ts.tv_sec - (int)tsleft.tv_sec, TIMERSEC);
		return PTS_FAIL;
	}

	return PTS_UNRESOLVED;
#else
	printf("CLOCK_MONOTONIC unsupported\n");
	return PTS_UNSUPPORTED;
#endif

}
