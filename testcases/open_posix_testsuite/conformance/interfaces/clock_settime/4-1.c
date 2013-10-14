/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test that if clock_settime() changes the value for CLOCK_REALTIME,
 * then any absolute timers will use the new time for expiration.
 *
 * Steps:
 * - get time T0
 * - create/enable a timer to expire at T1 = T0 + TIMEROFFSET
 * - sleep SLEEPTIME seconds (SLEEPTIME should be < TIMEROFFSET,
 *				but > ACCEPTABLEDELTA)
 * - set time back to T0
 * - wait for the timer to expire
 * - get time T2
 * - ensure that:  T2 >= T1 and (T2-T1) <= ACCEPTABLEDELTA
 *
 * signal SIGTOTEST is used.
 *
 * adam.li: I think should check that (abs(T2-T1) <= ACCEPTABLEDELTA)
 * 2004-04-30
 */
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include "posixtest.h"
#include "helpers.h"

// SLEEPTIME < TIMEROFFSET
// SLEEPTIME > ACCEPTABLEDELTA
#define SLEEPTIME 3
#define TIMEROFFSET 9
#define ACCEPTABLEDELTA 1

#define SIGTOTEST SIGALRM

int main(void)
{
	struct sigevent ev;
	struct timespec tpT0, tpT2, tpreset;
	struct itimerspec its;
	timer_t tid;
	int delta;
	int sig;
	sigset_t set;
	int flags = 0;

	/* Check that we're root...can't call clock_settime with CLOCK_REALTIME otherwise */
	if (geteuid() != 0) {
		printf("Run this test as ROOT, not as a Regular User\n");
		return PTS_UNTESTED;
	}

	/*
	 * set up sigevent for timer
	 * set up signal set for sigwait
	 */
	ev.sigev_notify = SIGEV_SIGNAL;
	ev.sigev_signo = SIGTOTEST;

	if (sigemptyset(&set) != 0) {
		perror("sigemptyset() was not successful\n");
		return PTS_UNRESOLVED;
	}

	if (sigaddset(&set, SIGTOTEST) != 0) {
		perror("sigaddset() was not successful\n");
		return PTS_UNRESOLVED;
	}

	if (sigprocmask(SIG_BLOCK, &set, NULL) == -1) {
		perror("sigprocmask() failed\n");
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
	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;
	its.it_value.tv_sec = tpT0.tv_sec + TIMEROFFSET;
	its.it_value.tv_nsec = tpT0.tv_nsec;
	if (timer_settime(tid, flags, &its, NULL) != 0) {
		perror("timer_settime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	sleep(SLEEPTIME);
	getBeforeTime(&tpreset);
	if (clock_settime(CLOCK_REALTIME, &tpT0) != 0) {
		perror("clock_settime() was not successful");
		return PTS_UNRESOLVED;
	}

	if (sigwait(&set, &sig) == -1) {
		perror("sigwait() was not successful\n");
		return PTS_UNRESOLVED;
	}

	if (clock_gettime(CLOCK_REALTIME, &tpT2) != 0) {
		printf("clock_gettime() was not successful\n");
		return PTS_UNRESOLVED;
	}

	delta = tpT2.tv_sec - its.it_value.tv_sec;

	// add back time waited to reset value and reset time
	tpreset.tv_sec += tpT2.tv_sec - tpT0.tv_sec;
	setBackTime(tpreset);

	printf("delta: %d\n", delta);
	if ((delta <= ACCEPTABLEDELTA) && (delta >= 0)) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	printf("FAIL:  Ended %d, not %d\n",
	       (int)tpT2.tv_sec, (int)its.it_value.tv_sec);
	return PTS_FAIL;
}
