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
 * Steps:
 * - Setup and then call timer_settime() with date in past.
 * - If timer_settime() returns -1 fail.
 * - If signal handler is not called after a sufficient amount of sleeping,
 *   fail.
 * ***  There is a potential false failure if timer_settime() does not
 *      return before the signal is sent.  TBD if this _should_ happen in
 *      reality, though.
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

#define DATEINPAST 1037128358	//Nov 13, 2002 ~11:13am

#define LONGSLEEPTIME 10

void handler(int signo LTP_ATTRIBUTE_UNUSED)
{
	printf("Caught signal\n");
	printf("Test PASSED\n");
	exit(PTS_PASS);
}

int main(void)
{
	struct sigevent ev;
	struct sigaction act;
	timer_t tid;
	struct itimerspec its;
	int flags = 0;

	ev.sigev_notify = SIGEV_SIGNAL;
	ev.sigev_signo = SIGTOTEST;

	act.sa_handler = handler;
	act.sa_flags = 0;

	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;
	its.it_value.tv_sec = DATEINPAST;
	its.it_value.tv_nsec = 0;

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

	flags |= TIMER_ABSTIME;
	if (timer_settime(tid, flags, &its, NULL) != 0) {
		perror("timer_settime() did not return success\n");
		return PTS_FAIL;
	}

	sleep(LONGSLEEPTIME);

	printf("signal was not sent\n");
	return PTS_FAIL;
}
