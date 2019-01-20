/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that nanosleep() sets rmtp to the time remaining if
 * it is interrupted by a signal.
 * If time remaining is within OKDELTA difference, the test is a pass.
 */
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "posixtest.h"

#define CHILDSUCCESS 1
#define CHILDFAILURE 0

#define OKDELTA 1

void handler(int signo LTP_ATTRIBUTE_UNUSED)
{
	printf("In handler\n");
}

int main(void)
{
	struct timespec tssleepfor, tsstorage, tsbefore, tsafter;
	int sleepsec = 30;
	int pid;
	struct sigaction act;

	if (clock_gettime(CLOCK_REALTIME, &tsbefore) == -1) {
		perror("Error in clock_gettime()\n");
		return PTS_UNRESOLVED;
	}

	if ((pid = fork()) == 0) {
		/* child here */
		int sleptplusremaining;

		act.sa_handler = handler;
		act.sa_flags = 0;
		if (sigemptyset(&act.sa_mask) == -1) {
			perror("Error calling sigemptyset\n");
			return CHILDFAILURE;
		}
		if (sigaction(SIGABRT, &act, 0) == -1) {
			perror("Error calling sigaction\n");
			return CHILDFAILURE;
		}
		tssleepfor.tv_sec = sleepsec;
		tssleepfor.tv_nsec = 0;
		if (nanosleep(&tssleepfor, &tsstorage) != -1) {
			printf("nanosleep() was not interrupted\n");
			return CHILDFAILURE;
		}

		if (clock_gettime(CLOCK_REALTIME, &tsafter) == -1) {
			perror("Error in clock_gettime()\n");
			return CHILDFAILURE;
		}

		sleptplusremaining = (tsafter.tv_sec - tsbefore.tv_sec) +
		    tsstorage.tv_sec;

		if (abs(sleptplusremaining - sleepsec) <= OKDELTA) {
			printf("PASS - within %d difference\n",
			       abs(sleptplusremaining - sleepsec));
			return CHILDSUCCESS;
		} else {
			printf("FAIL - within %d difference\n",
			       abs(sleptplusremaining - sleepsec));
			return CHILDFAILURE;
		}

	} else {
		/* parent here */
		int i;

		sleep(1);

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
