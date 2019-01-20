/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test that only a single timer expiration signal can be sent to
 * the process at a time.
 *
 * - Block signal SIGTOTEST.
 * - Set up a repeating timer to expire with signal SIGTOTEST.
 * - Sleep for enough time for > 2 signals to be sent.
 * - After the signals are unblocked, ensure only one signal is sent.
 */

#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "posixtest.h"

#define SIGTOTEST SIGALRM
#define TIMERVAL 2
#define TIMERINTERVAL 3

int madeit = 0;

void handler(int signo LTP_ATTRIBUTE_UNUSED)
{
	madeit++;
	if (madeit > 1) {
		printf(">1 signal made it through\n");
		exit(PTS_FAIL);
	}
}

int main(void)
{
	struct sigevent ev;
	struct sigaction act;
	timer_t tid;
	struct itimerspec its;
	int overruns;
	sigset_t set;

	ev.sigev_notify = SIGEV_SIGNAL;
	ev.sigev_signo = SIGTOTEST;

	act.sa_handler = handler;
	act.sa_flags = 0;

	/*
	 * set up handler for SIGTOTEST
	 */
	if (sigemptyset(&act.sa_mask) != 0) {
		perror("sigemptyset() did not return success\n");
		return PTS_UNRESOLVED;
	}
	if (sigaction(SIGTOTEST, &act, 0) != 0) {
		perror("sigaction() did not return success\n");
		return PTS_UNRESOLVED;
	}

	/*
	 * set up timer to send SIGTOTEST
	 */

	if (timer_create(CLOCK_REALTIME, &ev, &tid) != 0) {
		perror("timer_create() did not return success\n");
		return PTS_UNRESOLVED;
	}
	its.it_interval.tv_sec = TIMERINTERVAL;
	its.it_interval.tv_nsec = 0;
	its.it_value.tv_sec = TIMERVAL;
	its.it_value.tv_nsec = 0;

	if (timer_settime(tid, 0, &its, NULL) != 0) {
		perror("timer_settime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	/*
	 * block signal SIGTOTEST
	 */
	if (sigemptyset(&set) != 0) {
		perror("sigemptyset() did not return success\n");
		return PTS_UNRESOLVED;
	}
	if (sigaddset(&set, SIGTOTEST) != 0) {
		perror("sigaddset() did not return success\n");
		return PTS_UNRESOLVED;
	}
	if (sigprocmask(SIG_SETMASK, &set, NULL) != 0) {
		perror("sigprocmask() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if (sleep(2 * TIMERINTERVAL + TIMERVAL) != 0) {
		perror("Could not sleep for correct amount of time\n");
		return PTS_UNRESOLVED;
	}

	if (sigprocmask(SIG_UNBLOCK, &set, NULL) != 0) {
		perror("sigprocmask() did not return success\n");
		return PTS_UNRESOLVED;
	}

	overruns = timer_getoverrun(tid);
	printf("Total overruns: %d\n", overruns);
	if (madeit == 1) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	printf("FAIL:  %d signals sent\n", madeit);
	return PTS_FAIL;
}
