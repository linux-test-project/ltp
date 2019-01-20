/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test that if clock_settime() changes the value for CLOCK_REALTIME,
 * then any relative timers still expire when the time interval
 * has elapsed. [Test where clock_settime() sets timer forward in time.]
 *
 * Steps:
 * - set a timer to expire in TIMERSEC
 * - set the clock forward CLOCKOFFSET seconds
 * - nanosleep for TIMERSEC+SLEEPDELTA seconds -> timer should expire
 * - determine if the time remaining in nanosleep ~= SLEEPDELTA
 *
 * signal SIGTOTEST is used.
 */
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include "posixtest.h"
#include "helpers.h"

#define TIMERSEC 5
#define CLOCKOFFSET 4
#define SLEEPDELTA 3
#define ACCEPTABLEDELTA 1

#define SHORTTIME 1

#define SIGTOTEST SIGALRM

void handler(int signo)
{
	(void) signo;

	printf("Caught signal\n");
}

int main(void)
{
	struct sigevent ev;
	struct sigaction act;
	struct timespec tsclock, ts, tsleft, tsreset;
	struct itimerspec its;
	timer_t tid;
	sigset_t set;

	/* Check that we're root...can't call clock_settime with CLOCK_REALTIME otherwise */
	if (getuid() != 0) {
		printf("Run this test as ROOT, not as a Regular User\n");
		return PTS_UNTESTED;
	}

	/*
	 * set up sigevent for timer
	 * set up signal set for sigwait
	 * set up sigaction to catch signal
	 */
	ev.sigev_notify = SIGEV_SIGNAL;
	ev.sigev_signo = SIGTOTEST;

	act.sa_handler = handler;
	act.sa_flags = 0;

	if (sigemptyset(&set) != 0 || sigemptyset(&act.sa_mask) != 0) {
		perror("sigemptyset() was not successful\n");
		return PTS_UNRESOLVED;
	}

	if (sigaddset(&set, SIGTOTEST) != 0) {
		perror("sigaddset() was not successful\n");
		return PTS_UNRESOLVED;
	}

	if (sigaction(SIGTOTEST, &act, 0) != 0) {
		perror("sigaction() was not successful\n");
		return PTS_UNRESOLVED;
	}

	if (timer_create(CLOCK_REALTIME, &ev, &tid) != 0) {
		perror("timer_create() did not return success\n");
		return PTS_UNRESOLVED;
	}

	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;
	its.it_value.tv_sec = TIMERSEC;
	its.it_value.tv_nsec = 0;

	if (timer_settime(tid, 0, &its, NULL) != 0) {
		perror("timer_settime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if (clock_gettime(CLOCK_REALTIME, &tsclock) != 0) {
		printf("clock_gettime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	tsclock.tv_sec += CLOCKOFFSET;
	getBeforeTime(&tsreset);
	if (clock_settime(CLOCK_REALTIME, &tsclock) != 0) {
		printf("clock_settime() was not successful\n");
		return PTS_UNRESOLVED;
	}

	ts.tv_sec = TIMERSEC + SLEEPDELTA;
	ts.tv_nsec = 0;

	if (nanosleep(&ts, &tsleft) != -1) {
		printf("nanosleep() not interrupted\n");
		return PTS_FAIL;
	}

	if (labs(tsleft.tv_sec - SLEEPDELTA) <= ACCEPTABLEDELTA) {
		printf("Test PASSED\n");
		tsreset.tv_sec += TIMERSEC;
		setBackTime(tsreset);
		return PTS_PASS;
	}

	printf("Timer did not last for correct amount of time\n");
	printf("timer: %d != correct %d\n",
	       (int)ts.tv_sec - (int)tsleft.tv_sec, TIMERSEC);
	return PTS_FAIL;
}
