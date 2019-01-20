/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that if value.it_value = 0, the timer is disarmed.  Test by
 * disarming a currently armed and blocked timer.
 *
 * For this test, signal SIGTOTEST will be used, clock CLOCK_REALTIME
 * will be used.
 */

#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "posixtest.h"

#define TIMEREXPIRENSEC 10000000
#define SLEEPTIME 1

#define SIGTOTEST SIGALRM

void handler(int signo LTP_ATTRIBUTE_UNUSED)
{
	printf("OK to be in once\n");
}

int main(void)
{
	sigset_t set;
	struct sigevent ev;
	struct sigaction act;
	timer_t tid;
	struct itimerspec its;
	struct timespec ts;

	ev.sigev_notify = SIGEV_SIGNAL;
	ev.sigev_signo = SIGTOTEST;

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

	if (timer_create(CLOCK_REALTIME, &ev, &tid) != 0) {
		perror("timer_create() did not return success\n");
		return PTS_UNRESOLVED;
	}

	/*
	 * First set up timer to be blocked
	 */
	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;
	its.it_value.tv_sec = 0;
	its.it_value.tv_nsec = TIMEREXPIRENSEC;

	printf("setup first timer\n");
	if (timer_settime(tid, 0, &its, NULL) != 0) {
		perror("timer_settime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	printf("sleep\n");
	sleep(SLEEPTIME);
	printf("awoke\n");

	/*
	 * Second, set value.it_value = 0 and set up handler to catch
	 * signal.
	 */
	act.sa_handler = handler;
	act.sa_flags = 0;

	if (sigemptyset(&act.sa_mask) == -1) {
		perror("Error calling sigemptyset\n");
		return PTS_UNRESOLVED;
	}
	if (sigaction(SIGTOTEST, &act, 0) == -1) {
		perror("Error calling sigaction\n");
		return PTS_UNRESOLVED;
	}

	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;
	its.it_value.tv_sec = 0;
	its.it_value.tv_nsec = 0;

	printf("setup second timer\n");
	if (timer_settime(tid, 0, &its, NULL) != 0) {
		perror("timer_settime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	printf("unblock\n");
	if (sigprocmask(SIG_UNBLOCK, &set, NULL) != 0) {
		perror("sigprocmask() did not return success\n");
		return PTS_UNRESOLVED;
	}

	/*
	 * Ensure sleep for TIMEREXPIRE seconds not interrupted
	 */
	ts.tv_sec = SLEEPTIME;
	ts.tv_nsec = 0;

	printf("sleep again\n");
	if (nanosleep(&ts, NULL) == -1) {
		printf("nanosleep() interrupted\n");
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
