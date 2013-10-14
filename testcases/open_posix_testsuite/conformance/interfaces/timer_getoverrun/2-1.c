/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test that timer_getoverrun() returns the number of overruns that
 * have happened due to signals being sent from a timer.
 *
 * Steps (testing with just one overrun):
 * - Block signal SIGCONT (SIGCONT used so test will not terminate)
 * - Set up a timer to send SIGCONT on expiration with an interval
 *   of INTERVALSEC.
 * - Wait for that timer to expire twice (wait VALUESEC + INTERVALSEC).
 * - Call timer_getoverrun() and ensure 1 (EXPECTEDOVERRUNS) was returned.
 *   [First signal made it.  Second signal was the overrun.]
 */

#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include "posixtest.h"

#define VALUESEC 3
#define INTERVALSEC 4

#define EXPECTEDOVERRUNS 2

int main(void)
{
	sigset_t set;
	struct sigevent ev;
	timer_t tid;
	struct itimerspec its;
	int overruns;

	if (sigemptyset(&set) != 0) {
		perror("sigemptyset() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if (sigaddset(&set, SIGCONT) != 0) {
		perror("sigaddset() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if (sigprocmask(SIG_SETMASK, &set, NULL) != 0) {
		perror("sigprocmask() did not return success\n");
		return PTS_UNRESOLVED;
	}

	ev.sigev_notify = SIGEV_SIGNAL;
	ev.sigev_signo = SIGCONT;

	/*
	 * create first timer
	 */
	if (timer_create(CLOCK_REALTIME, &ev, &tid) != 0) {
		perror("timer_create() did not return success\n");
		return PTS_UNRESOLVED;
	}

	its.it_interval.tv_sec = INTERVALSEC;
	its.it_interval.tv_nsec = 0;
	its.it_value.tv_sec = VALUESEC;
	its.it_value.tv_nsec = 0;

	if (timer_settime(tid, 0, &its, NULL) != 0) {
		perror("timer_settime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	sleep(VALUESEC + 2 * INTERVALSEC + 1);

	if (sigprocmask(SIG_UNBLOCK, &set, NULL) != 0) {
		perror("sigprocmask() did not return success\n");
		return PTS_UNRESOLVED;
	}

	overruns = timer_getoverrun(tid);
	if (EXPECTEDOVERRUNS == overruns) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	printf("FAIL:  %d overruns sent; expected %d\n",
	       overruns, EXPECTEDOVERRUNS);
	return PTS_FAIL;
}
