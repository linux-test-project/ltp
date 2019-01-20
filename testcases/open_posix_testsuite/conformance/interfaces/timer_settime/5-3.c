/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that if timer_settime() is using an absolute clock and the
 * time has already taken place when the test is running that
 * timer_settime() succeeds and the expiration notification is made.
 *
 * Test for a variety of times which keep getting SUBTRACTAMOUNT
 * of time away from the time at the start of the test.
 * Note:  This test was made in response to a bug seen where timers
 *        would return -1 intermittently on time values generally
 *        > 9000 seconds before the current time.
 */

#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "posixtest.h"

#define SIGTOTEST SIGALRM

#define LONGSLEEPTIME 5

#define NUMTESTS 30

#define SUBTRACTAMOUNT 1000

int fails = 0, passes = 0;

void handler(int signo LTP_ATTRIBUTE_UNUSED)
{
	printf("Caught signal\n");
	passes += 1;
}

int main(void)
{
	struct sigevent ev;
	struct sigaction act;
	timer_t tid;
	struct itimerspec its;
	struct timespec ts;
	int flags = 0;
	int i;

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

	if (clock_gettime(CLOCK_REALTIME, &ts) != 0) {
		perror("clock_gettime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;
	its.it_value.tv_sec = ts.tv_sec;
	its.it_value.tv_nsec = 0;

	flags |= TIMER_ABSTIME;
	for (i = 0; i < NUMTESTS; i++) {
		if (timer_settime(tid, flags, &its, NULL) != 0) {
			perror("timer_settime() did not return success");
			printf("Error is %s\n", strerror(errno));
			printf("its.it_value.tv_sec was %d\n",
			       (int)its.it_value.tv_sec);
		} else {
			sleep(LONGSLEEPTIME);
		}
		its.it_value.tv_sec -= SUBTRACTAMOUNT;
	}
	fails = NUMTESTS - passes;

	printf("passes %d, fails %d\n", passes, fails);

	if (fails > 0)
		return PTS_FAIL;

	printf("Test PASSED\n");
	return PTS_PASS;
}
