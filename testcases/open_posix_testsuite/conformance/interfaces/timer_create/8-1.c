/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Copyright (c) 2002, Jim Houston.  All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * Patched by jim.houston REMOVE-THIS AT attbi DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that timers are not inherited across a fork().
 * Steps:
 * 1.  Set up a signal handler for timer in parent.
 * 2.  Create timer using timer_create().
 * 3.  Activate timer using timer_settime().
 * 4.  Immediately fork a new process [Note: There is some risk here if
 *     the system does not fork fast enough that there could be a false
 *     failure.  Times will be set large enough that this risk is minimized.]
 * 5.  Set up a signal handler for the timer in the parent and ensure it
 *     is not called by ensuring the child is able to sleep uninterrupted.
 *     [Note:  The delay to set up this handler could also cause false
 *     results.]
 *
 * For this test clock CLOCK_REALTIME will be used.
 *
 * 12/17/02 - Applied Jim Houston's patch that fixed a bug.  Parent originally
 *            was set to return PTS_UNRESOLVED if nanosleep() was interrupted,
 *            even though expected behavior is that it be interrupted once
 *            to catch the signal.
 */

#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "posixtest.h"

#define TIMERSEC 2
#define SLEEPDELTA 4
#define ACCEPTABLEDELTA 1

#define CHILDSUCCESS 1
#define CHILDFAILURE 0

void parenthandler(int signo LTP_ATTRIBUTE_UNUSED)
{
	printf("Expected - Caught signal\n");
}

void handler(int signo LTP_ATTRIBUTE_UNUSED)
{
	printf("Not expected - Caught signal\n");
}

int main(void)
{
	timer_t tid;
	struct sigaction actp;
	struct itimerspec its;
	int pid;

	actp.sa_handler = parenthandler;
	actp.sa_flags = 0;

	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;
	its.it_value.tv_sec = TIMERSEC;
	its.it_value.tv_nsec = 0;

	if (sigemptyset(&actp.sa_mask) == -1) {
		perror("Error calling sigemptyset\n");
		return PTS_UNRESOLVED;
	}
	if (sigaction(SIGALRM, &actp, 0) == -1) {
		perror("Error calling sigaction\n");
		return PTS_UNRESOLVED;
	}

	if (timer_create(CLOCK_REALTIME, NULL, &tid) != 0) {
		perror("timer_create() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if (timer_settime(tid, 0, &its, NULL) != 0) {
		perror("timer_settime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if ((pid = fork()) == 0) {
		/* child here */
		struct sigaction act;
		struct timespec ts, tsleft;
		act.sa_handler = handler;
		act.sa_flags = 0;

		if (sigemptyset(&act.sa_mask) == -1) {
			perror("Error calling sigemptyset\n");
			return CHILDFAILURE;
		}
		if (sigaction(SIGALRM, &act, 0) == -1) {
			perror("Error calling sigaction\n");
			return CHILDFAILURE;
		}

		ts.tv_sec = TIMERSEC + SLEEPDELTA;
		ts.tv_nsec = 0;

		if (nanosleep(&ts, &tsleft) == -1) {
			printf("child nanosleep() interrupted\n");
			return CHILDFAILURE;
		}
		//nanosleep() not interrupted
		return CHILDSUCCESS;

	} else {
		/* parent here */
		int i;
		struct timespec tsp, rem;

		/*
		 * parent also sleeps to allow timer to expire
		 */
		tsp.tv_sec = TIMERSEC;
		tsp.tv_nsec = 0;
		if (nanosleep(&tsp, &rem) == -1) {
			tsp = rem;
			if (nanosleep(&tsp, &rem) == -1) {
				printf("parent nanosleep() interrupted\n");
				return PTS_UNRESOLVED;
			}
		}

		if (wait(&i) == -1) {
			perror("Error waiting for child to exit\n");
			return PTS_UNRESOLVED;
		}
		if (WIFEXITED(i) && WEXITSTATUS(i)) {
			printf("Test PASSED\n");
			return PTS_PASS;
		} else {
			printf("Child did not exit normally.\n");
			printf("Test FAILED\n");
			return PTS_FAIL;
		}

	}

	return PTS_UNRESOLVED;
}
