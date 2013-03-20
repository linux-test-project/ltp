/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Note:  This test is not based on the POSIX spec, but is a speculation
 * about behavior for an assertion where the POSIX spec does not make a
 * statement about the behavior in either case.
 *
 * Test that if clock_settime() changes the value for CLOCK_REALTIME,
 * a repeating absolute timer uses this new value for expires.
 *
 * Document whether expirations which have already happened happen again or
 * not.
 *
 * Steps:
 * - get time T0
 * - create/enable a timer to expire at T1 = T0 + TIMERINTERVAL and repeat
 *   at interval TIMERINTERVAL
 * - sleep for time TIMERINTERVAL+ADDITIONALEXPIRES*TIMERINTERAL (must do
 *   in a loop to catch all expires)
 * - set time backward to T0
 * - sleep for time TIMERINTERVAL and ensure timer expires (2X)
 *
 * signal SIGTOTEST is used.
 */
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include "posixtest.h"
#include "../helpers.h"

#define TIMERINTERVAL 5
#define ADDITIONALEXPIRES 2

#define ADDITIONALDELTA 1

#define SIGTOTEST SIGALRM

int caught = 0;

void handler(int signo)
{
	(void) signo;

	printf("Caught signal\n");
	caught++;
}

int main(void)
{
	struct sigevent ev;
	struct sigaction act;
	struct timespec tpT0, tpclock, tsreset;
	struct itimerspec its;
	timer_t tid;
	int i, flags = 0, nocaught = 0;

	/*
	 * set up sigevent for timer
	 * set up sigaction to catch signal
	 */
	ev.sigev_notify = SIGEV_SIGNAL;
	ev.sigev_signo = SIGTOTEST;

	act.sa_handler = handler;
	act.sa_flags = 0;

	if (sigemptyset(&act.sa_mask) != 0) {
		perror("sigemptyset() was not successful\n");
		return PTS_UNRESOLVED;
	}

	if (sigaction(SIGTOTEST, &act, 0) != 0) {
		perror("sigaction() was not successful\n");
		return PTS_UNRESOLVED;
	}

	if (clock_gettime(CLOCK_REALTIME, &tpT0) != 0) {
		perror("clock_gettime() was not successful\n");
		return PTS_UNRESOLVED;
	}

	if (timer_create(CLOCK_REALTIME, &ev, &tid) != 0) {
		perror("timer_create() did not return success\n");
		return PTS_UNRESOLVED;
	}

	flags |= TIMER_ABSTIME;
	its.it_interval.tv_sec = TIMERINTERVAL;
	its.it_interval.tv_nsec = 0;
	its.it_value.tv_sec = tpT0.tv_sec + TIMERINTERVAL;
	its.it_value.tv_nsec = tpT0.tv_nsec;
	if (timer_settime(tid, flags, &its, NULL) != 0) {
		perror("timer_settime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	sleep(TIMERINTERVAL);
	for (i = 0; i < ADDITIONALEXPIRES; i++) {
		sleep(TIMERINTERVAL);
	}

	tpclock.tv_sec = tpT0.tv_sec;
	tpclock.tv_nsec = tpT0.tv_nsec;
	getBeforeTime(&tsreset);
	if (clock_settime(CLOCK_REALTIME, &tpclock) != 0) {
		printf("clock_settime() was not successful\n");
		return PTS_UNRESOLVED;
	}

	caught = 0;

	sleep(TIMERINTERVAL + ADDITIONALDELTA);

	if (caught == 1) {
		printf("Caught the first signal\n");
	} else {
		printf("FAIL:  Didn't catch timer after TIMERINTERVAL.\n");
		nocaught = 1;
	}

	sleep(TIMERINTERVAL + ADDITIONALDELTA);

	if (caught >= 2) {
		printf("Caught another signal\n");
	} else {
		printf("Caught %d < 2 signals\n", caught);
		nocaught = 1;
	}

	if (nocaught) {
		printf
		    ("Implementation does not repeat signals on clock reset\n");
	} else {
		printf("Implementation does repeat signals on clock reset\n");
	}

	// If we finish, pass
	tsreset.tv_sec += 2 * (TIMERINTERVAL + ADDITIONALDELTA);
	setBackTime(tsreset);
	return PTS_PASS;
}
