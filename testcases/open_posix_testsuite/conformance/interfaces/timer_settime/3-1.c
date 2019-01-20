/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that if value.it_value = 0, the timer is disarmed.
 * Steps:
 * 1.  Set up sigevent structure to send signal SIGTOTEST on timer
 *     expiration.
 * 2.  Set up sigaction structure to catch signal SIGTOTEST on timer
 *     expiration
 * 3.  Create timer using timer_create().
 * 4.  Activate timer using timer_settime() and then sleep.
 * 5.  Fail if signal handler is called.
 * 6.  If signal handler is not called, then PASS.
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

void handler(int signo LTP_ATTRIBUTE_UNUSED)
{
	printf("Incorrectly in signal handler\n");
	printf("Test FAILED\n");
	exit(PTS_FAIL);
}

int main(void)
{
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
	/*
	 * value.it_value = 0
	 */
	its.it_value.tv_sec = 0;
	its.it_value.tv_nsec = 0;

	ts.tv_sec = SLEEPDELTA;
	ts.tv_nsec = 0;

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

	if (timer_settime(tid, 0, &its, NULL) != 0) {
		perror("timer_settime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if (nanosleep(&ts, &tsleft) == -1) {
		printf("nanosleep() interrupted\n");
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
