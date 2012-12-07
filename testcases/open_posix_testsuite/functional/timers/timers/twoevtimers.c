/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test having two timers in different processes set to expire at the
 * same time, and ensure they both expire at the same time.
 */
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "posixtest.h"

#define EXPIREDELTA 3
#define LONGTIME 5

#define CHILDPASS 1

int caughtabort = 0;
int caughtalarm = 0;

void handler_abrt(int signo)
{
	printf("Caught abort signal\n");
	caughtabort++;
}

void handler_alrm(int signo)
{
	printf("Caught alarm signal\n");
	caughtalarm++;
}

int main(int argc, char *argv[])
{
	struct sigaction act1, act2;
	struct sigevent ev1, ev2;
	timer_t tid1, tid2;
	struct timespec ts;
	struct itimerspec its;
	int flags = 0;

	act1.sa_handler = handler_abrt;
	act1.sa_flags = 0;
	act2.sa_handler = handler_alrm;
	act2.sa_flags = 0;

	if ((sigemptyset(&act1.sa_mask) != 0) ||
	    (sigemptyset(&act2.sa_mask) != 0)) {
		perror("sigemptyset() did not return success\n");
		return PTS_UNRESOLVED;
	}
	if ((sigaction(SIGABRT, &act1, 0) != 0) ||
	    (sigaction(SIGALRM, &act2, 0) != 0)) {
		perror("sigaction() did not return success\n");
		return PTS_UNRESOLVED;
	}

	ev1.sigev_notify = SIGEV_SIGNAL;
	ev1.sigev_signo = SIGABRT;
	ev2.sigev_notify = SIGEV_SIGNAL;
	ev2.sigev_signo = SIGALRM;
	if ((timer_create(CLOCK_REALTIME, &ev1, &tid1) != 0) ||
	    (timer_create(CLOCK_REALTIME, &ev2, &tid2) != 0)) {
		perror("timer_create() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if (clock_gettime(CLOCK_REALTIME, &ts) != 0) {
		perror("clock_gettime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	its.it_value.tv_sec = ts.tv_sec + EXPIREDELTA;
	its.it_value.tv_nsec = ts.tv_nsec;
	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;

	flags |= TIMER_ABSTIME;
	if (timer_settime(tid1, flags, &its, NULL) != 0) {
		perror("timer_settime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if (timer_settime(tid2, flags, &its, NULL) != 0) {
		perror("timer_settime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	sleep(EXPIREDELTA + 1);

	if ((caughtalarm == 1) && (caughtabort == 1)) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else {
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	printf("This code should not be executed.\n");
	return PTS_UNRESOLVED;
}
