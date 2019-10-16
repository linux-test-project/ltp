/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Copyright (c) 2002, Jim Houston. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * Patched by:  jim.houston REMOVE-THIS AT attbi DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test that timer_getoverrun() returns the number of overruns that
 * have happened due to signals being sent from a timer.  Test with
 * timer seconds in smallest possible increments.
 *
 * valuensec = clock resolution
 * intervalnsec = 2*valuensec
 * expectedoverruns = (1,000,000,000 - valuensec) MOD intervalnsec
 *
 * Steps (testing with just one overrun):
 * - Block signal SIGCONT (SIGCONT used so test will not terminate)
 * - Set up a timer to send SIGCONT on expiration with an interval
 *   of intervalnsec nanoseconds.
 * - Wait for that timer to expire expectedoverruns+1 times
 *   (Sleep for valuensec + (expectedoverruns)*intervalnsec).
 * - Call timer_getoverrun() and ensure expectedoverruns was returned.
 *   [First signal made it.  All others were overruns.]
 *
 *   12/17/02 - Added Jim Houston's patch.  There is a chance that additional
 *   timer expires can happen before the overrun count is gotten, so this
 *   test stops the timer before that can happen.
 *
 *   04/29/2004 - adam.li
 *   - Add test for RTS option
 *   - It seems disalarm the timer before calling timer_getoverun() will discard
 *     previous overrun (when testing on libc-2004-04-29
 *   - Make itvalue = 1 sec.
 */

#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include "posixtest.h"

int main(void)
{
#ifndef _POSIX_REALTIME_SIGNALS
	printf("_POSIX_REALTIME_SIGNALS is not defined\n");
	return PTS_UNTESTED;
#endif
	sigset_t set;
	struct sigevent ev;
	timer_t tid;
	struct itimerspec its;
	struct timespec tssleep, tsres;
	int overruns;
	int valuensec, intervalnsec, expectedoverruns;
	int fudge;

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
		printf("Clock resolution in seconds, not nsecs.  Exiting.\n");
		return PTS_UNRESOLVED;
	}

	valuensec = tsres.tv_nsec;
	intervalnsec = 2 * valuensec;
	expectedoverruns = 1000000000 / intervalnsec - 1;

	/*
	 * waking up from sleep isn't instant, we can overshoot.
	 * Allow up to ~50ms worth of extra overruns.
	 */
	fudge = 50000000 / intervalnsec + 1;

	printf("value = %d sec, interval = %d nsec, "
	       "expected overruns = %d, fudge = %d\n", 1,
	       intervalnsec, expectedoverruns, fudge);

	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = intervalnsec;
	its.it_value.tv_sec = 1;
	its.it_value.tv_nsec = 0;
	//its.it_value.tv_sec = 0;
	//its.it_value.tv_nsec = valuensec;

	if (timer_settime(tid, 0, &its, NULL) != 0) {
		perror("timer_settime() did not return success\n");
		return PTS_UNRESOLVED;
	}
	//tssleep.tv_nsec = valuensec + (expectedoverruns*intervalnsec);
	tssleep.tv_nsec = 0;
	tssleep.tv_sec = 2;
	if (nanosleep(&tssleep, NULL) != 0) {
		perror("nanosleep() did not return success\n");
		return PTS_UNRESOLVED;
	}

	/*
	 * Since the overrun count is only meaningful with respect
	 * to a particular timer expiry disable the timer before
	 * un-blocking the signal.  This ensures that there is only
	 * one expiry and it should have a meaningful overrun count.
	 */
	//its.it_interval.tv_sec = 0;
	//its.it_interval.tv_nsec = 0;
	//its.it_value.tv_sec = 0;
	//its.it_value.tv_nsec = 0;
	//if (timer_settime(tid, 0, &its, NULL) != 0) {
	//      perror("timer_settime() did not return success\n");
	//      return PTS_UNRESOLVED;
	//}

	if (sigprocmask(SIG_UNBLOCK, &set, NULL) != 0) {
		perror("sigprocmask() did not return success\n");
		return PTS_UNRESOLVED;
	}

	overruns = timer_getoverrun(tid);
	printf("%d overruns occurred\n", overruns);
	/*
	 * Depending on the clock resolution we may have a few
	 * extra expiries after the nanosleep completes so do
	 * a range check.
	 */
	if (overruns >= expectedoverruns && overruns < expectedoverruns + fudge) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	printf("FAIL:  %d overruns sent; expected %d\n",
	       overruns, expectedoverruns);
	return PTS_FAIL;
}
