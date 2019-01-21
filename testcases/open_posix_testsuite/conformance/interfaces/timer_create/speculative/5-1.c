/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that the following scenarios are identical:
 * evp == NULL
 * and
 * evp.sigev_notify=SIGEV_SIGNAL
 * evp.sigev_signo="default signal" (assumed to be SIGALRM in speculative)
 * evp.sigev_value=timerid
 *
 * This is a speculative test because it is speculating on the value
 * of the default signal.
 *
 * Steps:
 * 1.  Set up sigaction structure to catch signal SIGALRM (default signal) on
 *     timer expiration
 * 2.  Create timer using timer_create().
 * 3.  Activate timer using timer_settime() and then sleep.
 * 4.  If signal handler is called, continue.  Otherwise, fail.
 * 5.  If signal handler was called and the time left in sleep ~= the
 *     delta between the timer time and sleep time, then PASS.
 *     Otherwise, FAIL.
 *
 * Note:  This test is identical to 1-1.c minus the evp lines.
 *
 * For this test clock CLOCK_REALTIME will be used.
 */

#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "posixtest.h"

#define DEFAULTSIG SIGALRM
#define TIMERSEC 2
#define SLEEPDELTA 3
#define ACCEPTABLEDELTA 1

void handler(int signo LTP_ATTRIBUTE_UNUSED)
{
	printf("Caught signal\n");
}

int main(void)
{
	struct sigaction act;
	timer_t tid;
	struct itimerspec its;
	struct timespec ts, tsleft;

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
	if (sigaction(DEFAULTSIG, &act, 0) == -1) {
		perror("Error calling sigaction\n");
		return PTS_UNRESOLVED;
	}

	if (timer_create(CLOCK_REALTIME, NULL, &tid) != 0) {
		perror("timer_create() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if (timer_settime(tid, 0, &its, NULL) != 0) {
		perror("timer_settime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if (nanosleep(&ts, &tsleft) != -1) {
		printf("nanosleep() not interrupted\n");
		printf("default signal most likely != SIGALRM\n");
		return PTS_PASS;
	}
	// Test can validly fail if timer did not last for correct amount
	// of time, but nanosleep() was interrupted.
	if (labs(tsleft.tv_sec - SLEEPDELTA) > ACCEPTABLEDELTA) {
		printf("Timer did not last for correct amount of time\n");
		printf("timer: %d != correct %d\n",
		       (int)ts.tv_sec - (int)tsleft.tv_sec, TIMERSEC);
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
