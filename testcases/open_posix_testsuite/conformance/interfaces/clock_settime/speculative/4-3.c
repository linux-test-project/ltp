/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test that if clock_settime() changes the value for CLOCK_REALTIME,
 * an absolute timer which would now have expired in the past
 * will expire immediately (with no error).
 * Test with a repeating absolute timer and set the clock forward past
 * > 1 expirations of that timer.
 *
 * Steps:
 * - get time T0
 * - create/enable a timer to expire at T1 = T0 + TIMEROFFSET and repeat
 *   at interval TIMERINTERVAL
 * - set time forward to T1 + EXPECTEDOVERRUNS*TIMERINTERVAL
 * - ensure that timer has expired with no error
 * - ensure that the overrun count is EXPECTEDOVERRUNS
 *   * Note:  The POSIX spec is unclear what exactly the overrun count
 *            should be in this case.  It is only speculation that it ==
 *            EXPECTEDOVERRUNS.  Either way passes.
 *            Information based on discussions with:
 *            benjamin.thery REMOVE-THIS AT bull DOT net
 *            george REMOVE-THIS AT mvista DOT com
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

#define TIMEROFFSET 3
#define TIMERINTERVAL 5
#define EXPECTEDOVERRUNS 3

#define SHORTTIME 1

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
	struct timespec tpT0, tpclock, tpreset;
	struct itimerspec its;
	timer_t tid;
	int flags = 0, overruns;

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
	its.it_value.tv_sec = tpT0.tv_sec + TIMEROFFSET;
	its.it_value.tv_nsec = tpT0.tv_nsec;
	if (timer_settime(tid, flags, &its, NULL) != 0) {
		perror("timer_settime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	tpclock.tv_sec = its.it_value.tv_sec + EXPECTEDOVERRUNS * TIMERINTERVAL;
	tpclock.tv_nsec = its.it_value.tv_nsec;
	getBeforeTime(&tpreset);
	if (clock_settime(CLOCK_REALTIME, &tpclock) != 0) {
		printf("clock_settime() was not successful\n");
		return PTS_UNRESOLVED;
	}

	sleep(SHORTTIME);

	overruns = timer_getoverrun(tid);
	if (overruns == EXPECTEDOVERRUNS) {
		printf("Overrun count == # of repeating timer expirys\n");
	} else {
		printf("Overrun count =%d, not # of repeating timer expirys\n",
		       overruns);
	}

	tpreset.tv_sec += SHORTTIME;
	setBackTime(tpreset);

	if (caught == 1) {
		printf("Caught the correct number of signals\n");
	} else {
		printf("FAIL:  Caught %d signals, not 1\n", caught);
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
