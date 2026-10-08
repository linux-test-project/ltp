/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test having two timers in the same process, armed with the same
 * absolute expiration time but delivering different signals, and ensure
 * that both of them expire.
 *
 * Note that the test does not (and cannot) verify that the two signals
 * are delivered simultaneously.  Signals are delivered one at a time, so
 * the second one may still be on its way when the first one arrives.
 * Poll for both of them in short sleeps until LONGTIME elapses.
 */
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "posixtest.h"
#include "timespec.h"

#define EXPIREDELTA_MS 50
#define EXPIREDELTA_NS (EXPIREDELTA_MS*1000000)
#define LONGTIME 3

#define POLL_MS 50
#define POLL_COUNT (LONGTIME * 1000 / POLL_MS)

#define CHILDPASS 1

static volatile sig_atomic_t caughtabort;
static volatile sig_atomic_t caughtalarm;

static void handler_abrt(int signo PTS_ATTRIBUTE_UNUSED)
{
	caughtabort++;
}

static void handler_alrm(int signo PTS_ATTRIBUTE_UNUSED)
{
	caughtalarm++;
}

int test_main(int argc, char **argv)
{
	struct sigaction act1, act2;
	struct sigevent ev1, ev2;
	timer_t tid1, tid2;
	struct timespec ts;
	struct itimerspec its;
	int flags = 0;
	struct timespec poll_ts = { .tv_nsec = POLL_MS * 1000000 };
	int i;

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

	its.it_value.tv_sec = ts.tv_sec;
	its.it_value.tv_nsec = ts.tv_nsec + EXPIREDELTA_NS;
	if (its.it_value.tv_nsec >= NSEC_IN_SEC) {
		its.it_value.tv_sec++;
		its.it_value.tv_nsec -= NSEC_IN_SEC;
	}
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

	/*
	 * Sleep in short chunks rather than once for LONGTIME: a signal
	 * that lands between the check and nanosleep() has nothing to
	 * interrupt, so a single long sleep would run out the whole budget.
	 * A chunk cut short by a signal only brings the next check forward.
	 */
	for (i = 0; i < POLL_COUNT && !(caughtalarm && caughtabort); i++) {
		if (nanosleep(&poll_ts, NULL) != 0 && errno != EINTR) {
			perror("nanosleep() did not return success");
			return PTS_UNRESOLVED;
		}
	}

	if (caughtabort)
		printf("Caught abort signal\n");
	if (caughtalarm)
		printf("Caught alarm signal\n");

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
