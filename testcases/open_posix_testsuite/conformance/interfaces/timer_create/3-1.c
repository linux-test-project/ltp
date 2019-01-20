/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that a timer is in the disarmed state when it is first created.
 *
 * Test by creating a timer, sleeping for SLEEPTIME, and ensuring
 * the sleep was not interrupted.
 *
 * For this test, CLOCK_REALTIME will be used.  Signal SIGTOTEST will be
 * used.
 */

#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "posixtest.h"

#define SIGTOTEST SIGALRM
#define SLEEPTIME 3

void handler(int signo LTP_ATTRIBUTE_UNUSED)
{
	printf("Caught signal\n");
}

int main(void)
{
	struct sigevent ev;
	struct sigaction act;
	timer_t tid;
	struct timespec ts;

	act.sa_handler = handler;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	if (sigaction(SIGTOTEST, &act, 0) == -1) {
		perror("Error calling sigaction\n");
		return PTS_UNRESOLVED;
	}

	ev.sigev_notify = SIGEV_SIGNAL;
	ev.sigev_signo = SIGTOTEST;
	if (timer_create(CLOCK_REALTIME, &ev, &tid) != 0) {
		perror("timer_create() did not return success\n");
		return PTS_FAIL;
	}

	ts.tv_sec = SLEEPTIME;
	ts.tv_nsec = 0;
	if (nanosleep(&ts, NULL) == -1) {
		perror("nanosleep() interrupted\n");
		return PTS_FAIL;
	}
	//Sleep not interrupted
	printf("Test PASSED\n");
	return PTS_PASS;
}
