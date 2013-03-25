/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test that timer_getoverrun() returns the number of overruns that
 * have happened due to signals being sent from a timer.  Test with
 * timer seconds in nanoseconds.
 *
 * INTERVALNSEC = clock resolution
 *
 * Steps (testing with just one overrun):
 * - Block signal SIGCONT (SIGCONT used so test will not terminate)
 * - Set up a timer to send SIGCONT on expiration with an interval
 *   of INTERVALNSEC nanoseconds.
 * - Wait for that timer to expire EXPECTEDIVERRUNS+1 times (wait VALUENSEC +
 *   (EXPECTEDIVERRUNS)*INTERVALNSEC).
 * - Call timer_getoverrun() and ensure EXPECTEDOVERRUNS was returned.
 *   [First signal made it.  All others were overruns.]
 */

#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include "posixtest.h"

#define VALUENSEC 2000000

#define EXPECTEDOVERRUNS 75

int main(void)
{
	sigset_t set;
	struct sigevent ev;
	timer_t tid;
	struct itimerspec its;
	struct timespec ts, tsres;
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

	if (clock_getres(CLOCK_REALTIME, &tsres) != 0) {
		perror("clock_gettime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if (tsres.tv_sec != 0) {
		printf("Clock resolution in seconds, not nsecs. Exiting.\n");
		return PTS_UNRESOLVED;
	}

	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = tsres.tv_nsec;
	its.it_value.tv_sec = 0;
	its.it_value.tv_nsec = VALUENSEC;

	if (timer_settime(tid, 0, &its, NULL) != 0) {
		perror("timer_settime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	ts.tv_nsec = VALUENSEC + ((EXPECTEDOVERRUNS) * its.it_interval.tv_nsec);
	ts.tv_sec = 0;
	if (nanosleep(&ts, NULL) != 0) {
		perror("nanosleep() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if (sigprocmask(SIG_UNBLOCK, &set, NULL) != 0) {
		perror("sigprocmask() did not return success\n");
		return PTS_UNRESOLVED;
	}

	overruns = timer_getoverrun(tid);
	if (overruns > EXPECTEDOVERRUNS - 5) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else {
		printf("FAIL:  %d overruns sent; expected %d\n",
		       overruns, EXPECTEDOVERRUNS);
		return PTS_FAIL;
	}
}
