/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that when clock_nanosleep() is interrupted by a signal, rmtp
 * contains the amount of time remaining (test with relative sleep).
 *
 */
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include "posixtest.h"
#include "proc.h"

#define SLEEPSEC 30

#define CHILDPASS 1
#define CHILDFAIL 0

#define OKDELTA 1

static void handler(int signo)
{
	(void) signo;

	printf("In handler\n");
}

int main(void)
{
	struct timespec tssleep, tsbefore, tsafter, tsremain;
	int pid;
	struct sigaction act;

	if (clock_gettime(CLOCK_REALTIME, &tsbefore) != 0) {
		perror("clock_gettime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if ((pid = fork()) == 0) {
		/* child here */
		int sleptplusremaining;

		act.sa_handler = handler;
		act.sa_flags = 0;
		if (sigemptyset(&act.sa_mask) != 0) {
			perror("sigemptyset() did not return success\n");
			return CHILDFAIL;
		}
		if (sigaction(SIGABRT, &act, 0) != 0) {
			perror("sigaction() did not return success\n");
			return CHILDFAIL;
		}
		tssleep.tv_sec = SLEEPSEC;
		tssleep.tv_nsec = 0;
		if (clock_nanosleep(CLOCK_REALTIME, 0,
				    &tssleep, &tsremain) == EINTR) {
			if (clock_gettime(CLOCK_REALTIME, &tsafter) != 0) {
				perror("clock_gettime() failed\n");
				return CHILDFAIL;
			}
			sleptplusremaining =
			    (tsafter.tv_sec - tsbefore.tv_sec) +
			    tsremain.tv_sec;

			if (abs(sleptplusremaining - SLEEPSEC) <= OKDELTA) {
				printf("PASS - within %d difference\n",
				       abs(sleptplusremaining - SLEEPSEC));
				return CHILDPASS;
			} else {
				printf("FAIL - within %d difference\n",
				       abs(sleptplusremaining - SLEEPSEC));
				return CHILDFAIL;
			}

			return CHILDFAIL;
		} else {
			printf("clock_nanosleep() did not return EINTR\n");
			return CHILDFAIL;
		}
	} else {
		/* parent here */
		int i;

		sleep(1);
		tst_process_state_wait3(pid, 'S', 1);

		if (kill(pid, SIGABRT) != 0) {
			printf("Could not raise signal being tested\n");
			return PTS_UNRESOLVED;
		}

		if (wait(&i) == -1) {
			perror("Error waiting for child to exit\n");
			return PTS_UNRESOLVED;
		}

		if (WIFEXITED(i) && WEXITSTATUS(i)) {
			printf("Child exited normally\n");
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
